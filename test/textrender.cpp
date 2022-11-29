#include <gawl/wayland/gawl.hpp>

#include "fc.hpp"

class Impl;

using Gawl = gawl::Gawl<Impl>;

class Impl {
  private:
    Gawl::Window<Impl>& window;
    gawl::TextRender    font;
    gawl::WrappedText   wrapped_text; // for cache

  public:
    auto refresh_callback() -> void {
        gawl::clear_screen({0, 0, 0, 1});

        gawl::draw_rect(window, {{0, 0}, {300, 40}}, {1, 1, 1, 1});
        font.draw_fit_rect(window, {{0, 0}, {300, 40}}, {0, 0, 0, 1}, "Hello, World!", 0, gawl::Align::Center, gawl::Align::Center);

        gawl::draw_rect(window, {{0, 50}, {300, 90}}, {1, 1, 1, 1});
        font.draw_fit_rect(window, {{0, 50}, {300, 90}}, {0, 0, 0, 1}, "Hello, World!", 0, gawl::Align::Right, gawl::Align::Center);

        gawl::draw_rect(window, {{0, 100}, {300, 140}}, {1, 1, 1, 1});
        font.draw_fit_rect(window, {{0, 100}, {300, 140}}, {0, 0, 0, 1}, "Hello, World!", 0, gawl::Align::Left, gawl::Align::Center);

        gawl::draw_rect(window, {{0, 150}, {300, 190}}, {1, 1, 1, 1});
        font.draw_fit_rect(window, {{0, 150}, {300, 190}}, {0, 0, 0, 1}, "Hello, World!", 0, gawl::Align::Center, gawl::Align::Right);

        gawl::draw_rect(window, {{0, 200}, {300, 240}}, {1, 1, 1, 1});
        font.draw_fit_rect(window, {{0, 200}, {300, 240}}, {0, 0, 0, 1}, "Hello, World!", 0, gawl::Align::Center, gawl::Align::Left);

        constexpr auto poem = "雨ニモマケズ\n風ニモマケズ\n雪ニモ夏ノ暑サニモマケヌ\n丈夫ナカラダヲモチ\n慾ハナク\n決シテ瞋ラズ\nイツモシヅカニワラッテヰル\n一日ニ玄米四合ト\n味噌ト少シノ野菜ヲタベ\nアラユルコトヲ\nジブンヲカンジョウニ入レズニ\nヨクミキキシワカリ\nソシテワスレズ\n野原ノ松ノ林ノ䕃ノ\n小サナ萱ブキノ小屋ニヰテ\n東ニ病気ノコドモアレバ\n行ッテ看病シテヤリ\n西ニツカレタ母アレバ\n行ッテソノ稲ノ束ヲ負ヒ\n南ニ死ニサウナ人アレバ\n行ッテコハガラナクテモイヽトイヒ\n北ニケンクヮヤソショウガアレバ\nツマラナイカラヤメロトイヒ\nヒデリノトキハナミダヲナガシ\nサムサノナツハオロオロアルキ\nミンナニデクノボートヨバレ\nホメラレモセズ\nクニモサレズ\nサウイフモノニ\nワタシハナリタイ";
        gawl::draw_rect(window, {{310, 0}, {610, 640}}, {1, 1, 1, 1});
        font.draw_wrapped(window, {{310, 0}, {610, 640}}, 18, {0, 0, 0, 1}, poem, wrapped_text, 16);
    }

    Impl(Gawl::Window<Impl>& window) : window(window), font({fc::find_fontpath_from_name("Noto Sans CJK JP").data()}, 32) {}
};

auto main() -> int {
    auto app = Gawl::Application();
    app.open_window<Impl>({.manual_refresh = false});
    app.run();
    return 0;
}
