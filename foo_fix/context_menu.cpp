#include "stdafx.h"

#include "context_menu.h"
#include "wav_tag_fixer.h"

void my_context_menu::get_item_name(unsigned p_index, pfc::string_base& p_out) {
    switch(p_index) {
        case  ITEM_INDEX_WAVTAGFIX:
            p_out = pfc::stringcvt::string_utf8_from_wide(L"WAV��ǩ�޸�");
            break;

        default:
            uBugCheck();
    }
}

void my_context_menu::context_command(unsigned p_index, metadb_handle_list_cref p_data, const GUID& p_caller) {
    switch(p_index) {
        case ITEM_INDEX_WAVTAGFIX:
        {
            if(::MessageBoxW(core_api::get_main_window(),
                             L"�������ͼ�޸�Foobar2000д�����ЧRIFF��Ϣ��.\n\n"\
                             L"�޸������п��������Ƶ�ļ��𻵣�������߲��Դ���ɵ���ʧ����.\n�����б��ݻ�����޸�!\n"\
                             L"\n"\
                             L"ȷ��������",
                             L"����",
                             MB_OKCANCEL | MB_ICONWARNING) == IDOK) {

                wav_tag_fixer fixer;
                fixer.fix(p_data);
            }
        }
        break;

        default:
            uBugCheck();
    }
}

bool my_context_menu::context_get_display(unsigned p_index, metadb_handle_list_cref p_data, pfc::string_base& p_out, unsigned& p_displayflags, const GUID& p_caller) {
    PFC_ASSERT(p_index >= 0 && p_index < get_num_items());
    get_item_name(p_index, p_out);
    return true;
}

GUID my_context_menu::get_item_guid(unsigned p_index) {
    switch(p_index) {
        case ITEM_INDEX_WAVTAGFIX:
            return g_wavtagfixer_context_menu_guid;

        default:
            uBugCheck();
    }
}

namespace {
    static contextmenu_item_factory_t<my_context_menu> g_my_context_menu_factory;
}