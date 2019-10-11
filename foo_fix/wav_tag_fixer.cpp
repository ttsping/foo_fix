#include "stdafx.h"
#include "wav_tag_fixer.h"

FOURCC wav_tag_fixer::wav_tag_fix_threaded_process_callback::get_riff_info_id(const char* meta_name) {
    struct {
        const char* meta;
        FOURCC ckid;
    }riff_list_info_remappings[] = {
        { "title", RIFFINFO_INAM } ,
        { "artist", RIFFINFO_IART } ,
        { "album", RIFFINFO_IPRD } ,
        { "tracknumber", RIFFINFO_ITRK } ,
        { "genre", RIFFINFO_IGNR } ,
        { "date", RIFFINFO_ICRD } ,
        { "comment", RIFFINFO_ICMT } ,
        { "copyright", RIFFINFO_ICOP } ,
    };

    for(t_size n = 0; n < pfc::array_size_t(riff_list_info_remappings); ++n) {
        if(_stricmp(meta_name, riff_list_info_remappings[n].meta) == 0) {
            return riff_list_info_remappings[n].ckid;
        }
    }
    return 0;
}

foobar2000_io::t_filesize wav_tag_fixer::wav_tag_fix_threaded_process_callback::locate_chunk(file_ptr& file, FOURCC ckid, abort_callback& abort) {
    t_filesize offset = filesize_invalid;
    t_filesize pos = file->get_position(abort);
    for(;;) {
        if(file->is_eof(abort))
            break;

        t_chunk chunk;
        file->read_object(&chunk, sizeof(chunk), abort);
        if(chunk.chunk_id == ckid) {
            offset = file->get_position(abort) - sizeof(chunk);
            break;
        }
        file->seek_ex(chunk.chunk_size, file::seek_from_current, abort);
        if(chunk.chunk_size & 0x01) {
            file->seek_ex(1, file::seek_from_current, abort);
        }
    }
    file->seek(pos, abort);
    return offset;
}

void wav_tag_fixer::wav_tag_fix_threaded_process_callback::run(threaded_process_status& p_status, abort_callback& p_abort) {
    pfc::string_formatter log;

    t_size n, m = m_data.get_count();
    for(n = 0; n < m; ++n) {

        if(p_abort.is_aborting())
            break;

        metadb_handle_ptr& meta = m_data[n];

        // check extension
        pfc::string_extension ext(meta->get_path());
        if(_stricmp(ext, "wav") != 0)
            continue;

        // set process bar status
        file_path_display display_path(meta->get_path());
        p_status.set_item(display_path);
        p_status.set_progress(n, m);

        // fixed me: foobar2000 always writes LIST chunk in the end of file?
        try {

            file_ptr wav_file;
            filesystem::g_open(wav_file, display_path, filesystem::open_mode_write_existing, p_abort);
            wav_file->ensure_local();

            t_chunk chunk;
            wav_file->read(&chunk, sizeof(chunk), p_abort);
            if(chunk.chunk_id != FOURCC_RIFF)
                continue;

            DWORD wave_id;
            wav_file->read_object_t(wave_id, p_abort);
            if(wave_id != MAKEFOURCC('W', 'A', 'V', 'E'))
                continue;

            // locate LIST -> INFO sub-chunk
            t_filesize list_postion = locate_chunk(wav_file, MAKEFOURCC('L', 'I', 'S', 'T'), p_abort);
            if(list_postion == filesize_invalid)
                continue;
            wav_file->seek(list_postion, p_abort);
            // skip LIST chunk header
            wav_file->seek_ex(sizeof(t_chunk), file::seek_from_current, p_abort);

            // check if INFO type LIST
            DWORD info_type;
            wav_file->read_object_t(info_type, p_abort);
            if(info_type != MAKEFOURCC('I', 'N', 'F', 'O'))
                continue;

            // locate id3 chunk which written by foobar2000
            t_filesize id3_position = locate_chunk(wav_file, MAKEFOURCC('i', 'd', '3', ' '), p_abort);
            if(id3_position == filesize_invalid)
                continue;

            // backup id3 chunk
            wav_file->seek(id3_position, p_abort);
            wav_file->read_object(&chunk, sizeof(chunk), p_abort);
            wav_file->seek(id3_position, p_abort);
            pfc::array_t<t_uint8> id3_chunk;
            id3_chunk.set_size(chunk.chunk_size + sizeof(chunk));
            wav_file->read(id3_chunk.get_ptr(), id3_chunk.get_size(), p_abort);

            // re-write LIST chunk
            wav_file->seek(list_postion, p_abort);
            wav_file->seek_ex(sizeof(t_chunk), file::seek_from_current, p_abort);

            info_type = MAKEFOURCC('I', 'N', 'F', 'O');
            wav_file->write(&info_type, sizeof(info_type), p_abort);

            t_size info_chunk_size = sizeof(FOURCC); // 'INFO'
            const file_info& meta_info = meta->get_info_ref()->info();
            for(t_size meta_walk = 0; meta_walk < meta_info.meta_get_count(); ++meta_walk) {
                FOURCC ckid = get_riff_info_id(meta_info.meta_enum_name(meta_walk));
                if(ckid == 0) continue;
                pfc::string8 temp8;
                meta_info.meta_format_entry(meta_walk, temp8);
                pfc::stringcvt::string_ansi_from_utf8 temp(temp8);
                // write INFO sub-chunk
                chunk.chunk_id = ckid;
                chunk.chunk_size = temp.length() + 1;
                wav_file->write(&chunk, sizeof(chunk), p_abort);
                wav_file->write(temp.get_ptr(), chunk.chunk_size, p_abort);
                if(chunk.chunk_size & 0x00000001) { // padding 1 byte when length is odd
                    char char_null = 0;
                    wav_file->write(&char_null, 1, p_abort);
                    info_chunk_size += 1;
                }
                info_chunk_size += sizeof(chunk) + chunk.chunk_size;
            }

            // re-write id3 chunk
            wav_file->write(id3_chunk.get_ptr(), id3_chunk.get_size(), p_abort);
            wav_file->set_eof(p_abort);

            // fix LIST chunk size
            wav_file->seek(list_postion + sizeof(FOURCC), p_abort);
            wav_file->write_lendian_t(info_chunk_size, p_abort);

            // fix RIFF chunk size
            wav_file->seek(sizeof(FOURCC), p_abort);
            t_size riff_chunk_size = (t_size)wav_file->get_size(p_abort) - sizeof(t_chunk);
            wav_file->write_lendian_t(riff_chunk_size, p_abort);

        } catch(pfc::exception & e) {
            log << "fix " << display_path << " failed: \r\n" << e.what() << "\r\n";
        }
    }
}

void wav_tag_fixer::wav_tag_fix_threaded_process_callback::on_done(HWND p_wnd, bool p_was_aborted) {

}

bool wav_tag_fixer::fix(metadb_handle_list_cref p_data) {

    service_ptr_t<wav_tag_fix_threaded_process_callback> task = new service_impl_t<wav_tag_fix_threaded_process_callback>(p_data);

    threaded_process::g_run_modeless(task,
                                     threaded_process::flag_show_abort |
                                     threaded_process::flag_show_item |
                                     threaded_process::flag_show_progress |
                                     threaded_process::flag_no_focus |
                                     threaded_process::flag_show_delayed,
                                     core_api::get_main_window(),
                                     pfc::stringcvt::string_utf8_from_wide(L"ÐÞ¸´ÖÐ..."));

    return true;
}


