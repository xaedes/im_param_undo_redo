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
        class GuiBackend_t = im_param::ImGuiBackend,
        class SerializerBackend_t = im_param::JsonSerializerBackend,
        class DeserializerBackend_t = im_param::JsonDeserializerBackend
    >
    struct UndoRedoSerializingBackend {
        bool changed = false;

        using GuiBackend = GuiBackend_t;
        using SerializerBackend = SerializerBackend_t;
        using DeserializerBackend = DeserializerBackend_t;
        template <class... Args> using TypeHolder = typename im_param::TypeHolder<Args...>;
        using HierarchyType = typename im_param::HierarchyType;
        using Backend = typename im_param::Backend;

        UndoRedoSerializingBackend()
        {}

        GuiBackend gui_backend;
        SerializerBackend serializer_backend;
        DeserializerBackend deserializer_backend;
        
        LinearHistory<std::string> history;

        template <class value_type, class... Args>
        void setDefault(const std::string& name, value_type& value, Args&&... args)
        {
            history.default_value(serialize(name, value, std::forward<Args>(args)...));
        }

        template <class value_type, class... Args>
        std::string serialize(const std::string& name, value_type& value, Args&&... args)
        {
            serializer_backend.clear();
            serializer_backend.parameter(name, value, std::forward<Args>(args)...);
            return serializer_backend.serialized();;
        }

        template <class value_type, class... Args>
        bool deserialize(const std::string& serialized, const std::string& name, value_type& value, Args&&... args)
        {
            deserializer_backend.clear();
            deserializer_backend.changed = false;
            deserializer_backend.deserialize(serialized);
            deserializer_backend.parameter(name, value, std::forward<Args>(args)...);
            return deserializer_backend.changed;
        }

        // template<class... Args, std::enable_if_t<Backend::is_specialized<value_type>::value, bool> = true>
        template<class value_type, class... Args/*, std::enable_if_t<Backend::is_specialized<value_type>::value, bool> = true*/>
        UndoRedoSerializingBackend& parameter(const std::string& name, value_type& value, Args&&... args)
        {
            this->begin(name, value, std::forward<Args>(args)...);
            gui_backend.changed = false;
            bool changed_by_user = gui_backend.parameter(
                name, value, std::forward<Args>(args)...
            ).changed;
            this->end(changed_by_user, name, value, std::forward<Args>(args)...);
            return *this;
        }

        // template<typename U, class... Args, std::enable_if_t<!Backend::is_specialized<value_type>::value, bool> = true>
        template<class value_type, typename U, class... Args /*, std::enable_if_t<!Backend::is_specialized<value_type>::value, bool> = true */>
        UndoRedoSerializingBackend& parameter(const std::string& name, value_type& value, const TypeHolder<U>& typeholder, Args&&... args)
        {
            this->begin(name, value, typeholder, typeholder, std::forward<Args>(args)...);
            gui_backend.changed = false;
            bool changed_by_user = gui_backend.parameter(
                name, value, typeholder, std::forward<Args>(args)...
            ).changed;
            this->end(changed_by_user, name, value, typeholder, std::forward<Args>(args)...);
            return *this;
        }

        template <class value_type, class... Args>
        void begin(const std::string& name, value_type& value, Args&&... args)
        {
            if (ImGuiButton("Default"))
            {
                if (history.reset_to_default())
                {
                    deserialize(history.current_value(), name, value, std::forward<Args>(args)...);
                    changed |= true;
                }
            }
            ImGui::SameLine();
            if (ImGuiButton("Undo", history.undo_available() > 0))
            {
                if (history.undo())
                {
                    deserialize(history.current_value(), name, value, std::forward<Args>(args)...);
                    changed |= true;
                }
            }
            ImGui::SameLine();
            if (ImGuiButton("Redo", history.redo_available() > 0))
            {
                if (history.redo())
                {
                    deserialize(history.current_value(), name, value, std::forward<Args>(args)...);
                    changed |= true;
                }
            }
        }

        template <class value_type, class... Args>
        void end(bool changed_by_user, const std::string& name, value_type& value, Args&&... args)
        {
            if (changed_by_user)
            {
                auto serizalized = serialize(name, value, std::forward<Args>(args)...);
                history.push_back(serizalized);
                changed |= true;
            }

        }

    };
    
} // namespace im_param_undo_redo
