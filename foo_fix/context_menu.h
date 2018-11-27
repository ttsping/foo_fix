#pragma once


class my_context_menu : public contextmenu_item_simple
{
	enum t_item_index
	{
		ITEM_INDEX_WAVTAGFIX = 0 ,
	};

public:
	virtual GUID get_parent() { return contextmenu_groups::tagging; }
	virtual t_enabled_state get_enabled_state(unsigned p_index) { return DEFAULT_OFF; }
	virtual unsigned get_num_items() { return 1; }
	virtual void get_item_name(unsigned p_index,pfc::string_base & p_out);
	virtual void context_command(unsigned p_index,metadb_handle_list_cref p_data,const GUID& p_caller);
	virtual bool context_get_display(unsigned p_index,metadb_handle_list_cref p_data,pfc::string_base & p_out,unsigned & p_displayflags,const GUID & p_caller);
	virtual GUID get_item_guid(unsigned p_index);
	virtual bool get_item_description(unsigned p_index,pfc::string_base & p_out) { return false; }
};