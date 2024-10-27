#include "gawl/textrender.hpp"
#include "gawl/fc.hpp"
#include "gawl/misc.hpp"
#include "gawl/wayland/application.hpp"
#include "macros/unwrap.hpp"
#include "util/assert.hpp"

class Callbacks : public gawl::WindowCallbacks {
  private:
    gawl::TextRender  font;
    gawl::WrappedText wrapped_text; // for cache

  public:
    auto refresh() -> void override {
        gawl::clear_screen({0, 0, 0, 1});

        gawl::draw_rect(*window, {{0, 0}, {300, 40}}, {1, 1, 1, 1});
        font.draw_fit_rect(*window, {{0, 0}, {300, 40}}, {0, 0, 0, 1}, "Hello, World!", 0, gawl::Align::Center, gawl::Align::Center);

        gawl::draw_rect(*window, {{0, 50}, {300, 90}}, {1, 1, 1, 1});
        font.draw_fit_rect(*window, {{0, 50}, {300, 90}}, {0, 0, 0, 1}, "Hello, World!", 0, gawl::Align::Right, gawl::Align::Center);

        gawl::draw_rect(*window, {{0, 100}, {300, 140}}, {1, 1, 1, 1});
        font.draw_fit_rect(*window, {{0, 100}, {300, 140}}, {0, 0, 0, 1}, "Hello, World!", 0, gawl::Align::Left, gawl::Align::Center);

        gawl::draw_rect(*window, {{0, 150}, {300, 190}}, {1, 1, 1, 1});
        font.draw_fit_rect(*window, {{0, 150}, {300, 190}}, {0, 0, 0, 1}, "Hello, World!", 0, gawl::Align::Center, gawl::Align::Right);

        gawl::draw_rect(*window, {{0, 200}, {300, 240}}, {1, 1, 1, 1});
        font.draw_fit_rect(*window, {{0, 200}, {300, 240}}, {0, 0, 0, 1}, "Hello, World!", 0, gawl::Align::Center, gawl::Align::Left);

        constexpr auto poem = "雨ニモマケズ\n風ニモマケズ\n雪ニモ夏ノ暑サニモマケヌ\n丈夫ナカラダヲモチ\n慾ハナク\n決シテ瞋ラズ\nイツモシヅカニワラッテヰル\n一日ニ玄米四合ト\n味噌ト少シノ野菜ヲタベ\nアラユルコトヲ\nジブンヲカンジョウニ入レズニ\nヨクミキキシワカリ\nソシテワスレズ\n野原ノ松ノ林ノ䕃ノ\n小サナ萱ブキノ小屋ニヰテ\n東ニ病気ノコドモアレバ\n行ッテ看病シテヤリ\n西ニツカレタ母アレバ\n行ッテソノ稲ノ束ヲ負ヒ\n南ニ死ニサウナ人アレバ\n行ッテコハガラナクテモイヽトイヒ\n北ニケンクヮヤソショウガアレバ\nツマラナイカラヤメロトイヒ\nヒデリノトキハナミダヲナガシ\nサムサノナツハオロオロアルキ\nミンナニデクノボートヨバレ\nホメラレモセズ\nクニモサレズ\nサウイフモノニ\nワタシハナリタイ";

        {
            const auto area = gawl::Rectangle{{310, 0}, {610, 640}};
            gawl::draw_rect(*window, area, {1, 1, 1, 1});
            font.draw_wrapped(*window, area, 18, {0, 0, 0, 1}, poem, wrapped_text, 16, gawl::Align::Left, gawl::Align::Left);
        }

        {
            const auto area = gawl::Rectangle{{620, 0}, {920, 640}};
            gawl::draw_rect(*window, area, {1, 1, 1, 1});
            font.draw_wrapped(*window, area, 18, {0, 0, 0, 1}, poem, wrapped_text, 16, gawl::Align::Center, gawl::Align::Center);
        }

        {
            const auto area = gawl::Rectangle{{930, 0}, {1230, 640}};
            gawl::draw_rect(*window, area, {1, 1, 1, 1});
            font.draw_wrapped(*window, area, 18, {0, 0, 0, 1}, poem, wrapped_text, 16, gawl::Align::Right, gawl::Align::Right);
        }
    }

    auto close() -> void override {
        application->quit();
        return void();
    }

    auto on_created(gawl::Window* /*window*/) -> coop::Async<bool> override {
        constexpr auto error_value = false;

        co_unwrap_v_mut(fontpath, gawl::find_fontpath_from_name("Noto Sans CJK JP"));
        font.init({std::move(fontpath)}, 32);
        co_return true;
    }
};

auto main() -> int {
    auto runner = coop::Runner();
    auto app    = gawl::WaylandApplication();
    auto cbs    = std::shared_ptr<Callbacks>(new Callbacks());
    runner.push_task(app.run(), app.open_window({.manual_refresh = true}, std::move(cbs)));
    runner.run();
}
