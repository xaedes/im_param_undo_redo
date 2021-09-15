#pragma once

#include <vector>
#include <string>

namespace im_param_undo_redo {

    template <class value_t>
    struct LinearHistory
    {
        using value_type = value_t;

        LinearHistory(const value_type& default_value = value_type(), size_t max_size = 100)
            : m_history{default_value}
            , m_index(0)
            , m_maxSize(max_size)
        {}

        void clear()
        {
            m_history.resize(1);
            m_index = 0;
        }

        void push_back(const value_type& value)
        {
            m_history.push_back(value);
            if ((m_maxSize > 0) && (size()-1 > m_maxSize))
            {
                auto num_erase = size() - 1 - m_maxSize;
                auto erase_begin = m_history.begin() + 1;
                m_history.erase(
                    erase_begin,
                    erase_begin + num_erase
                );
            }
            m_index = m_history.size()-1;
        }

        bool reset_to_default()
        {
            if (m_index > 0) 
            {
                m_index = 0;
                return true;
            }
            return false;
        }

        bool undo()
        {
            if (undo_available() > 0)
            {
                --m_index;
                return true;
            }
            return false;
        }

        bool redo()
        {
            if (redo_available() > 0)
            {
                ++m_index;
                return true;
            }
            return false;
        }

        void clear(value_type& value)
        {
            clear();
            value = current_value();
        }

        bool reset_to_default(value_type& value)
        {
            bool result = reset_to_default();
            if (result)
            {
                value = current_value();
            }
            return result;
        }

        bool undo(value_type& value)
        {
            bool result = undo();
            if (result)
            {
                value = current_value();
            }
            return result;
        }

        bool redo(value_type& value)
        {
            bool result = redo();
            if (result)
            {
                value = current_value();
            }
            return result;
        }

        size_t undo_available() const
        {
            return m_index;
        }
        
        size_t redo_available() const
        {
            return size() - 1 - m_index;
        }

        void default_value(const value_type& val) { m_history[0] = val; }
        const value_type& default_value() const { return m_history[0]; }

        value_type& current_value() { return m_history[m_index]; }
        const value_type& current_value() const { return m_history[m_index]; }

    protected:

        size_t size() const { return m_history.size(); }

        std::vector<value_type>& items() {  return m_history; }
        const std::vector<value_type>& items() const {  return m_history; }


        std::vector<value_type> m_history;
        size_t m_index;
        size_t m_maxSize;

    };

} // namespace im_param_undo_redo
