
#ifndef __SHIFTJISDATA_H__
#define __SHIFTJISDATA_H__

namespace shiftjis_data {
    
    namespace messages {
        //"%s%s　%s月%s日%s時%s分" in shift-jis
        extern const unsigned char datetime[23];

        //"%s%s　−−−−−−−−−−−−" in shift-jis
        extern const unsigned char spacing[31];

        //"%s%sにセーブします。よろしいですか？" in shift-jis
        extern const unsigned char saving[37];

        //"%s%sをロードします。よろしいですか？" in shift-jis
        extern const unsigned char loading[37];

        //"リセットします。よろしいですか？" in shift-jis
        extern const unsigned char return_to_title[33];

        //"終了します。よろしいですか？" in shift-jis
        extern const unsigned char quit[29];

        //"はい" in shift-jis
        extern const unsigned char yes[5];

        //"いいえ" in shift-jis
        extern const unsigned char no[7];
    }
    namespace paths {
        // "/System/Library/Fonts/繝偵Λ繧ｭ繧吶ヮ荳ｸ繧ｳ繧? ProN W4.ttc" in shift-jis
        extern const unsigned char hiragino_shiftjis[59];
    }
    namespace kinsoku_defaults {
        // "」』）］｝、。，．・？！ヽヾゝゞ々ー" in shift-jis
        extern const unsigned char start_kinsoku[37];

        // "「『（［｛" in shift-jis
        extern const unsigned char end_kinsoku[11];
    }

    namespace inserts {
        // "?ｿｽ?ｿｽ" in shift-jis encoding.
        extern const unsigned char questions[7];

        // "０" in shift-jis
        extern const unsigned char wide_zero[3];

        // "　" in shift-jis
        extern const unsigned char wide_space[3];

        // "−" in shift-jis
        extern const unsigned char wide_dash[3];

        // "０１２３４５６７８９" in shift-jis
        extern const unsigned char wide_numbers[21];
    }

    namespace menu_labels {
        // "＜セーブ＞" in shift-jis
        extern const unsigned char save_menu_name[11];

        // "＜ロード＞" in shift-jis
        extern const unsigned char load_menu_name[11];

        // "しおり" in shift-jis
        extern const unsigned char save_item_name[7];
    }   
}

#endif
