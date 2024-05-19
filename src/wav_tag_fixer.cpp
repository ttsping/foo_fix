#include "pch.h"
#include "wav_tag_fixer.h"
#include <taglib/wavfile.h>

void wav_tag_fixer::wav_tag_fix_threaded_process_callback::run(threaded_process_status& p_status, abort_callback& p_abort)
{
    for (size_t idx = 0, metaCount = m_metadb_list.get_count(); idx < metaCount; ++idx)
    {
        const metadb_handle_ptr& meta = m_metadb_list[idx];
        const file_info& info = m_file_info_list[idx];

        if (meta->get_subsong_index() != 0)
        {
            continue;
        }

        pfc::string8 display_path = file_path_display(meta->get_path());
        try
        {
            if (_stricmp(pfc::string_extension(display_path), "wav") != 0)
            {
                continue;
            }

            // set process bar status
            p_status.set_item(display_path);
            p_status.set_progress(idx, metaCount);

            process_item(meta, info);
        }
        catch (std::exception& e)
        {
            FB2K_console_formatter() << "Processing " << display_path << " exception: " << e.what();
        }
        catch (...)
        {
        }
    }
}

void wav_tag_fixer::wav_tag_fix_threaded_process_callback::process_item(const metadb_handle_ptr& meta, const file_info& info)
{
    std::wstring fileNameWide(pfc::stringcvt::string_wide_from_utf8(file_path_display(meta->get_path())));
    TagLib::RIFF::WAV::File wavFile(TagLib::FileName(fileNameWide.c_str()));
    if (wavFile.isValid())
    {
        if (!wavFile.hasInfoTag())
        {
            return;
        }

        auto infoTag = wavFile.InfoTag();
        pfc::string8 title, artist, album, comment, genre;
        if (info.meta_format("TITLE", title))
        {
            infoTag->setTitle(TagLib::String(pfc::stringcvt::string_ansi_from_utf8(title)));
        }

        if (info.meta_format("ARTIST", artist))
        {
            infoTag->setArtist(TagLib::String(pfc::stringcvt::string_ansi_from_utf8(artist)));
        }

        if (info.meta_format("ALBUM", album))
        {
            infoTag->setAlbum(TagLib::String(pfc::stringcvt::string_ansi_from_utf8(album)));
        }

        if (info.meta_format("COMMENT", comment))
        {
            infoTag->setComment(TagLib::String(pfc::stringcvt::string_ansi_from_utf8(comment)));
        }

        if (info.meta_format("GENRE", genre))
        {
            infoTag->setGenre(TagLib::String(pfc::stringcvt::string_ansi_from_utf8(genre)));
        }

        wavFile.save(TagLib::RIFF::WAV::File::Info, TagLib::File::StripNone);
    }
}

bool wav_tag_fixer::fix(metadb_handle_list_cref p_data, t_infosref p_info)
{
    service_ptr_t<wav_tag_fix_threaded_process_callback> task = new service_impl_t<wav_tag_fix_threaded_process_callback>(p_data, p_info);
    threaded_process::g_run_modeless(task,
                                     threaded_process::flag_show_abort | threaded_process::flag_show_item | threaded_process::flag_show_progress |
                                         threaded_process::flag_no_focus | threaded_process::flag_show_delayed,
                                     core_api::get_main_window(), pfc::stringcvt::string_utf8_from_wide(L"ÐÞ¸´ÖÐ..."));

    return true;
}
