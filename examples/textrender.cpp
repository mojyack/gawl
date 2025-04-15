#include "gawl/textrender.hpp"
#include "gawl/fc.hpp"
#include "gawl/misc.hpp"
#include "gawl/wayland/application.hpp"
#include "macros/unwrap.hpp"

constexpr auto poem = R"(雨ニモマケズ
風ニモマケズ
雪ニモ夏ノ暑サニモマケヌ
丈夫ナカラダヲモチ
慾ハナク
決シテ瞋ラズ
イツモシヅカニワラッテヰル
一日ニ玄米四合ト
味噌ト少シノ野菜ヲタベ
アラユルコトヲ
ジブンヲカンジョウニ入レズニ
ヨクミキキシワカリ
ソシテワスレズ
野原ノ松ノ林ノ䕃ノ
小サナ萱ブキノ小屋ニヰテ
東ニ病気ノコドモアレバ
行ッテ看病シテヤリ
西ニツカレタ母アレバ
行ッテソノ稲ノ束ヲ負ヒ
南ニ死ニサウナ人アレバ
行ッテコハガラナクテモイヽトイヒ
北ニケンクヮヤソショウガアレバ
ツマラナイカラヤメロトイヒ
ヒデリノトキハナミダヲナガシ
サムサノナツハオロオロアルキ
ミンナニデクノボートヨバレ
ホメラレモセズ
クニモサレズ
サウイフモノニ
ワタシハナリタイ)";

class Callbacks : public gawl::WindowCallbacks {
  private:
    gawl::TextRender  font;
    gawl::WrappedText wrapped_text; // for cache

  public:
    auto refresh() -> void override {
        gawl::clear_screen({0, 0, 0, 1});

        gawl::draw_rect(*window, {{0, 0}, {300, 40}}, {1, 1, 1, 1});
        font.draw_fit_rect(*window, {{0, 0}, {300, 40}}, {0, 0, 0, 1}, "Hello, World!", {
                                                                                            .align_x = gawl::Align::Center,
                                                                                            .align_y = gawl::Align::Center,
                                                                                        });

        gawl::draw_rect(*window, {{0, 50}, {300, 90}}, {1, 1, 1, 1});
        font.draw_fit_rect(*window, {{0, 50}, {300, 90}}, {0, 0, 0, 1}, "Hello, World!", {
                                                                                             .align_x = gawl::Align::Right,
                                                                                             .align_y = gawl::Align::Center,
                                                                                         });

        gawl::draw_rect(*window, {{0, 100}, {300, 140}}, {1, 1, 1, 1});
        font.draw_fit_rect(*window, {{0, 100}, {300, 140}}, {0, 0, 0, 1}, "Hello, World!", {
                                                                                               .align_x = gawl::Align::Left,
                                                                                               .align_y = gawl::Align::Center,
                                                                                           });

        gawl::draw_rect(*window, {{0, 150}, {300, 190}}, {1, 1, 1, 1});
        font.draw_fit_rect(*window, {{0, 150}, {300, 190}}, {0, 0, 0, 1}, "Hello, World!", {
                                                                                               .align_x = gawl::Align::Center,
                                                                                               .align_y = gawl::Align::Right,
                                                                                           });

        gawl::draw_rect(*window, {{0, 200}, {300, 240}}, {1, 1, 1, 1});
        font.draw_fit_rect(*window, {{0, 200}, {300, 240}}, {0, 0, 0, 1}, "Hello, World!", {
                                                                                               .align_x = gawl::Align::Center,
                                                                                               .align_y = gawl::Align::Left,
                                                                                           });

        {
            const auto area = gawl::Rectangle{{310, 0}, {610, 640}};
            gawl::draw_rect(*window, area, {1, 1, 1, 1});
            font.draw_wrapped(*window, area, 18, {0, 0, 0, 1}, poem, wrapped_text, {
                                                                                       .size    = 16,
                                                                                       .align_x = gawl::Align::Left,
                                                                                       .align_y = gawl::Align::Left,
                                                                                   });
        }

        {
            const auto area = gawl::Rectangle{{620, 0}, {920, 640}};
            gawl::draw_rect(*window, area, {1, 1, 1, 1});
            font.draw_wrapped(*window, area, 18, {0, 0, 0, 1}, poem, wrapped_text, {
                                                                                       .size    = 16,
                                                                                       .align_x = gawl::Align::Center,
                                                                                       .align_y = gawl::Align::Center,
                                                                                   });
        }

        {
            const auto area = gawl::Rectangle{{930, 0}, {1230, 640}};
            gawl::draw_rect(*window, area, {1, 1, 1, 1});
            font.draw_wrapped(*window, area, 18, {0, 0, 0, 1}, poem, wrapped_text, {
                                                                                       .size    = 16,
                                                                                       .align_x = gawl::Align::Right,
                                                                                       .align_y = gawl::Align::Right,
                                                                                   });
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
    runner.push_task(app.run());
    runner.push_task(app.open_window({.manual_refresh = true}, std::move(cbs)));
    runner.run();
}
