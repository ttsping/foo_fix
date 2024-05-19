#pragma once
#include <mmreg.h>

class wav_tag_fixer
{
  private:
    class file_info_list : public pfc::list_t<file_info_impl>
    {
      private:
        typedef pfc::list_base_const_t<const file_info*> t_interface;

      public:
        file_info_list(const t_interface& p_source)
        {
            for (auto entry : p_source)
            {
                this->add_item(*entry);
            }
        }
    };

    typedef const pfc::list_base_const_t<const file_info*>& t_infosref;

  private:
    class wav_tag_fix_threaded_process_callback : public threaded_process_callback
    {
      public:
        wav_tag_fix_threaded_process_callback(metadb_handle_list_cref p_data, t_infosref p_info) : m_metadb_list(p_data), m_file_info_list(p_info)
        {
        }

        void on_init(HWND p_wnd) override
        {
        }
        //! Called from the worker thread. Do all the hard work here.
        void run(threaded_process_status& p_status, abort_callback& p_abort) override;
        //! Called after the worker thread has finished executing.
        void on_done(HWND p_wnd, bool p_was_aborted) override
        {
        }

      private:
        void process_item(const metadb_handle_ptr& meta, const file_info& info);

      private:
        metadb_handle_list m_metadb_list;
        file_info_list m_file_info_list;
    };

  public:
    static bool fix(metadb_handle_list_cref p_data, t_infosref p_info);
};