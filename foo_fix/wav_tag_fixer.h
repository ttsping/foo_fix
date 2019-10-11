#pragma once
#include <mmreg.h>

class wav_tag_fixer
{
private:
    class wav_tag_fix_threaded_process_callback : public threaded_process_callback
    {

    private:

#pragma pack(push,1)
        struct t_chunk
        {
            FOURCC chunk_id;
            DWORD chunk_size;
        };
#pragma pack(pop)

    private:

        FOURCC get_riff_info_id(const char* meta_name);

        t_filesize locate_chunk(file_ptr& file, FOURCC ckid, abort_callback& abort);

    public:

        wav_tag_fix_threaded_process_callback(metadb_handle_list_cref p_data) : m_data(p_data) {}

        virtual void on_init(HWND p_wnd) {}
        //! Called from the worker thread. Do all the hard work here.
        virtual void run(threaded_process_status& p_status, abort_callback& p_abort);
        //! Called after the worker thread has finished executing.
        virtual void on_done(HWND p_wnd, bool p_was_aborted);

    private:
        metadb_handle_list m_data;
    };

public:
    bool fix(metadb_handle_list_cref p_data);
};