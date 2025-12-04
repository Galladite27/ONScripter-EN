#include <cstring>

#include "ShiftJISData.h"

#include "gtest/gtest.h"

// clang doesn't like shift-jis encoded files, so we only test on gcc, which should be good enough, except maybe for the macos only string.
#if defined(__GNUC__) && !defined(__clang__)

TEST (TestShiftJISData, Strings) {
    EXPECT_EQ(sizeof("?øΩ?øΩ"), sizeof(shiftjis_data::inserts::questions));
    EXPECT_TRUE(0 == memcmp("?øΩ?øΩ", shiftjis_data::inserts::questions, sizeof(shiftjis_data::inserts::questions)));
    EXPECT_EQ(sizeof("%s%sÅ@%såé%sì˙%séû%sï™"), sizeof(shiftjis_data::messages::datetime));
    EXPECT_TRUE(0 == memcmp("%s%sÅ@%såé%sì˙%séû%sï™", shiftjis_data::messages::datetime, sizeof(shiftjis_data::messages::datetime)));
    EXPECT_EQ(sizeof("%s%sÅ@Å|Å|Å|Å|Å|Å|Å|Å|Å|Å|Å|Å|"), sizeof(shiftjis_data::messages::spacing));
    EXPECT_TRUE(0 == memcmp("%s%sÅ@Å|Å|Å|Å|Å|Å|Å|Å|Å|Å|Å|Å|", shiftjis_data::messages::spacing, sizeof(shiftjis_data::messages::spacing)));
    EXPECT_EQ(sizeof("%s%sÇ…ÉZÅ[ÉuÇµÇ‹Ç∑ÅBÇÊÇÎÇµÇ¢Ç≈Ç∑Ç©ÅH"), sizeof(shiftjis_data::messages::saving));
    EXPECT_TRUE(0 == memcmp("%s%sÇ…ÉZÅ[ÉuÇµÇ‹Ç∑ÅBÇÊÇÎÇµÇ¢Ç≈Ç∑Ç©ÅH", shiftjis_data::messages::saving, sizeof(shiftjis_data::messages::saving)));
    EXPECT_EQ(sizeof("%s%sÇÉçÅ[ÉhÇµÇ‹Ç∑ÅBÇÊÇÎÇµÇ¢Ç≈Ç∑Ç©ÅH"), sizeof(shiftjis_data::messages::loading));
    EXPECT_TRUE(0 == memcmp("%s%sÇÉçÅ[ÉhÇµÇ‹Ç∑ÅBÇÊÇÎÇµÇ¢Ç≈Ç∑Ç©ÅH", shiftjis_data::messages::loading, sizeof(shiftjis_data::messages::loading)));
    EXPECT_EQ(sizeof("ÉäÉZÉbÉgÇµÇ‹Ç∑ÅBÇÊÇÎÇµÇ¢Ç≈Ç∑Ç©ÅH"), sizeof(shiftjis_data::messages::return_to_title));
    EXPECT_TRUE(0 == memcmp("ÉäÉZÉbÉgÇµÇ‹Ç∑ÅBÇÊÇÎÇµÇ¢Ç≈Ç∑Ç©ÅH", shiftjis_data::messages::return_to_title, sizeof(shiftjis_data::messages::return_to_title)));
    EXPECT_EQ(sizeof("èIóπÇµÇ‹Ç∑ÅBÇÊÇÎÇµÇ¢Ç≈Ç∑Ç©ÅH"), sizeof(shiftjis_data::messages::quit));
    EXPECT_TRUE(0 == memcmp("èIóπÇµÇ‹Ç∑ÅBÇÊÇÎÇµÇ¢Ç≈Ç∑Ç©ÅH", shiftjis_data::messages::quit, sizeof(shiftjis_data::messages::quit)));
    EXPECT_EQ(sizeof("ÇÕÇ¢"), sizeof(shiftjis_data::messages::yes));
    EXPECT_TRUE(0 == memcmp("ÇÕÇ¢", shiftjis_data::messages::yes, sizeof(shiftjis_data::messages::yes)));
    EXPECT_EQ(sizeof("Ç¢Ç¢Ç¶"), sizeof(shiftjis_data::messages::no));
    EXPECT_TRUE(0 == memcmp("Ç¢Ç¢Ç¶", shiftjis_data::messages::no, sizeof(shiftjis_data::messages::no)));
    EXPECT_EQ(sizeof("/System/Library/Fonts/„Éí„É©„Ç≠„Çô„Éé‰∏∏„Ç≥„Ç? ProN W4.ttc"), sizeof(shiftjis_data::paths::hiragino_shiftjis));
    EXPECT_TRUE(0 == memcmp("/System/Library/Fonts/„Éí„É©„Ç≠„Çô„Éé‰∏∏„Ç≥„Ç? ProN W4.ttc", shiftjis_data::paths::hiragino_shiftjis, sizeof(shiftjis_data::paths::hiragino_shiftjis)));
    EXPECT_EQ(sizeof("ÅvÅxÅjÅnÅpÅAÅBÅCÅDÅEÅHÅIÅRÅSÅTÅUÅXÅ["), sizeof(shiftjis_data::kinsoku_defaults::start_kinsoku));
    EXPECT_TRUE(0 == memcmp("ÅvÅxÅjÅnÅpÅAÅBÅCÅDÅEÅHÅIÅRÅSÅTÅUÅXÅ[", shiftjis_data::kinsoku_defaults::start_kinsoku, sizeof(shiftjis_data::kinsoku_defaults::start_kinsoku)));
    EXPECT_EQ(sizeof("ÅuÅwÅiÅmÅo"), sizeof(shiftjis_data::kinsoku_defaults::end_kinsoku));
    EXPECT_TRUE(0 == memcmp("ÅuÅwÅiÅmÅo", shiftjis_data::kinsoku_defaults::end_kinsoku, sizeof(shiftjis_data::kinsoku_defaults::end_kinsoku)));
    EXPECT_EQ(sizeof("ÇO"), sizeof(shiftjis_data::inserts::wide_zero));
    EXPECT_TRUE(0 == memcmp("ÇO", shiftjis_data::inserts::wide_zero, sizeof(shiftjis_data::inserts::wide_zero)));
    EXPECT_EQ(sizeof("Å@"), sizeof(shiftjis_data::inserts::wide_space));
    EXPECT_TRUE(0 == memcmp("Å@", shiftjis_data::inserts::wide_space, sizeof(shiftjis_data::inserts::wide_space)));
    EXPECT_EQ(sizeof("Å|"), sizeof(shiftjis_data::inserts::wide_dash));
    EXPECT_TRUE(0 == memcmp("Å|", shiftjis_data::inserts::wide_dash, sizeof(shiftjis_data::inserts::wide_dash)));
    EXPECT_EQ(sizeof("ÇOÇPÇQÇRÇSÇTÇUÇVÇWÇX"), sizeof(shiftjis_data::inserts::wide_numbers));
    EXPECT_TRUE(0 == memcmp("ÇOÇPÇQÇRÇSÇTÇUÇVÇWÇX", shiftjis_data::inserts::wide_numbers, sizeof(shiftjis_data::inserts::wide_numbers)));
    EXPECT_EQ(sizeof("ÅÉÉZÅ[ÉuÅÑ"), sizeof(shiftjis_data::menu_labels::save_menu_name));
    EXPECT_TRUE(0 == memcmp("ÅÉÉZÅ[ÉuÅÑ", shiftjis_data::menu_labels::save_menu_name, sizeof(shiftjis_data::menu_labels::save_menu_name)));
    EXPECT_EQ(sizeof("ÅÉÉçÅ[ÉhÅÑ"), sizeof(shiftjis_data::menu_labels::load_menu_name));
    EXPECT_TRUE(0 == memcmp("ÅÉÉçÅ[ÉhÅÑ", shiftjis_data::menu_labels::load_menu_name, sizeof(shiftjis_data::menu_labels::load_menu_name)));
    EXPECT_EQ(sizeof("ÇµÇ®ÇË"), sizeof(shiftjis_data::menu_labels::save_item_name));
    EXPECT_TRUE(0 == memcmp("ÇµÇ®ÇË", shiftjis_data::menu_labels::save_item_name, sizeof(shiftjis_data::menu_labels::save_item_name)));
}

#endif
