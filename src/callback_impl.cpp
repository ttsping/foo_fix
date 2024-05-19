#include "pch.h"
#include <taglib/infotag.h>
#include "wav_tag_fixer.h"

class TagLibRIFFInfoStringHandler : public TagLib::RIFF::Info::StringHandler
{
public:
    TagLib::String parse(const TagLib::ByteVector& data) const override
    {
        return TagLib::String(data, TagLib::String::Latin1);
    }

    TagLib::ByteVector render(const TagLib::String& s) const override
    {
        return s.data(TagLib::String::Latin1);
    }
};

TagLibRIFFInfoStringHandler tagLibRIFFInfoStringHandler;

class metadb_io_edit_callback_impl : public metadb_io_edit_callback
{
  public:
    void on_edited(metadb_handle_list_cref items, t_infosref before, t_infosref after) override
    {
        TagLib::RIFF::Info::Tag::setStringHandler(&tagLibRIFFInfoStringHandler);
        wav_tag_fixer::fix(items, after);
    }
};

namespace {
    static service_factory_single_t<metadb_io_edit_callback_impl> g_metadb_io_edit_callback_impl;
}