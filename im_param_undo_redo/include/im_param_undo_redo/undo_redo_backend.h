#pragma once

#include "imgui.h"
#include "imgui_candy/imgui_candy.h"

#include "im_param/detail/cpp11_template_logic.h"
#include "im_param/detail/backend.h"
#include "im_param/backends/imgui_backend.h"
#include "im_param/backends/json_backend.h"
#include "im_param_undo_redo/linear_history.h"

namespace im_param_undo_redo {

    template<
        class T,
        class GuiBackend_t = im_param::ImGuiBackend,
        class SerializerBackend_t = im_param::JsonSerializerBackend,
        class DeserializerBackend_t = im_param::JsonDeserializerBackend
    >
    struct UndoRedoBackend {
        bool changed = false;

        using value_type = T;
        using GuiBackend = GuiBackend_t;
        using SerializerBackend = SerializerBackend_t;
        using DeserializerBackend = DeserializerBackend_t;
        template <class... Args> using TypeHolder = typename im_param::TypeHolder<Args...>;
        using HierarchyType = typename im_param::HierarchyType;
        using Backend = typename im_param::Backend;

        UndoRedoBackend()
        {}

        GuiBackend gui_backend;
        
        LinearHistory<value_type> history;

        // template<class... Args, std::enable_if_t<Backend::is_specialized<value_type>::value, bool> = true>
        template<class... Args/*, std::enable_if_t<Backend::is_specialized<value_type>::value, bool> = true*/>
        UndoRedoBackend& parameter(const std::string& name, value_type& value, Args&&... args)
        {
            begin_gui(value);
            gui_backend.changed = false;
            bool changed_by_user = gui_backend.parameter(
                name, value, std::forward<Args>(args)...
            ).changed;
            end_gui(value, changed_by_user);
            return *this;
        }

        // template<typename U, class... Args, std::enable_if_t<!Backend::is_specialized<value_type>::value, bool> = true>
        template<typename U, class... Args /*, std::enable_if_t<!Backend::is_specialized<value_type>::value, bool> = true */>
        UndoRedoBackend& parameter(const std::string& name, value_type& value, const TypeHolder<U>& typeholder, Args&&... args)
        {
            begin_gui(value);
            gui_backend.changed = false;
            bool changed_by_user = gui_backend.parameter(
                name, value, typeholder, std::forward<Args>(args)...
            ).changed;
            end_gui(value, changed_by_user);
            return *this;
        }

        void begin_gui(value_type& value)
        {
            if (ImGuiButton("Default"))
            {
                changed |= history.reset_to_default(value);
            }
            ImGui::SameLine();
            if (ImGuiButton("Undo", history.undo_available() > 0))
            {
                changed |= history.undo(value);
            }
            ImGui::SameLine();
            if (ImGuiButton("Redo", history.redo_available() > 0))
            {
                changed |= history.redo(value);
            }
        }

        void end_gui(value_type& value, bool changed_by_user = false)
        {
            if (changed_by_user)
            {
                history.push_back(value);
                changed |= true;
            }

        }

    };
    
} // namespace im_param_undo_redo
