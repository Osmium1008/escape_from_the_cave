#include <Siv3D.hpp>

enum Status { _start, _select, _tutorial, _course, _game_finish, _submit, _ranking };

class FontManager {
	Array<Font> fontset;

  public:
	FontManager(int max_size) : fontset(max_size, Font(0)){};
	Rect draw_center(int font_size, String text, Vec2 pos, ColorF color = ColorF(1.0, 1.0, 1.0), double rect_size = 1.0, Color rect_color = Color(0x0f, 0x13, 0x29)) {
		if (fontset.at(font_size - 1) == Font(0)) fontset[font_size - 1] = Font(font_size);
		fontset.at(font_size - 1)(text).region(Arg::center = pos).scaled(rect_size).draw(rect_color);
		return fontset.at(font_size - 1)(text).draw(Arg::center = pos, color);
	}
};

class Player {
	Rect body;

	Vec2 speed;

	Vec2 last_pos;

	bool up, down, right, left;

	Stopwatch stopwatch;
	double last_jump;

	int item_number;
	Array<Vec2> position_using_item;

  public:
	Player() : body(150, 500, 30, 70), speed(0, 0), up(false), down(false), right(false), left(false) {
		stopwatch.start();
		last_pos = body.center();
	}

	void move(int a) {
		speed.x += a;
	}

	void jump(int a) {
		if (down) {
			last_jump = stopwatch.sF();
		}
		if (stopwatch.sF() - last_jump < -0.5) speed.y -= a;
	}

	bool update() {
		last_pos = body.center();
		body.moveBy(speed.asPoint());
		speed.y += 1;
		speed.y = 0;
		body.draw(Palette::White);
		if (death()) {
			if (position_using_item) {
				Vec2 point = position_using_item.back();
				position_using_item.pop_back();
			}
			else {
				return false;
			}
		}
		return true;
	}

	bool death() {
		return up && down || right && left;
	}

	void setIntersects(bool arg_up, bool arg_down, bool arg_right, bool arg_left) {
		up = arg_up;
		down = arg_down;
		left = arg_left;
		right = arg_right;
	}
	Vec2 getLastPos() {
		return last_pos;
	}

	Vec2 getPos() {
		return body.center();
	}
};

class Course {};

void Main() {
	Status status = _start;
	int status_number = _tutorial;

	FontManager font(60);
	Scene::SetBackground(Color(0x0f, 0x13, 0x29));

	std::chrono::duration<double> clock = 2.0s;
	Stopwatch time;

	Window::Resize(800, 600);
	Player player;

	while (System::Update()) {
		ClearPrint();
		double delta = 200 * Scene::DeltaTime();

		switch (status) {
			case _start:
				font.draw_center(60, U"Escape from the cave", Scene::Center() / Vec2(1.0, 2.0));
				font.draw_center(30, U"---- ↓ キーを押してください ----", Scene::Center() * Vec2(1.0, 1.5) + Vec2(0, 40), ColorF(1.0, 1.0, 1.0, Periodic::Square0_1(clock)));
				font.draw_center(30, U" 2019, 制作者:Osmium_1008 ", Scene::Center() * Vec2(1.0, 1.5) + Vec2(0, 90));
				if (KeyDown.down()) {
					clock = 0.2s;
					time.start();
				}
				if (time.sF() > 0.5) {
					status = _select;
					time.reset();
				}
				break;
			case _select:
				font.draw_center(45, U" このゲームを遊ぶのは初めてですか？", Scene::Center() / Vec2(1.0, 2.0) - Vec2(0, 70));
				font.draw_center(30, U" ←→キー:選択, ↓キー:決定", Scene::Center() / Vec2(1.0, 2.0));
				font.draw_center(30, U" はい ", Scene::Center() * Vec2(0.5, 1.5), ColorF(0, 0, 0), 1.5, status_number == _tutorial ? Color(160, 216, 239, time.isRunning() ? Periodic::Square0_1(0.2) * 255 : 255) : Color(255, 255, 255));
				font.draw_center(30, U"いいえ", Scene::Center() * Vec2(1.5, 1.5), ColorF(0, 0, 0), 1.5, status_number == _course ? Color(160, 216, 239, time.isRunning() ? Periodic::Square0_1(0.2) * 255 : 255) : Color(255, 255, 255));
				if (KeyRight.down()) status_number = _course;
				if (KeyLeft.down()) status_number = _tutorial;
				if (KeyDown.down()) {
					clock = 0.2s;
					time.start();
				}
				if (time.sF() > 0.5) {
					status = (Status)status_number;
					status_number = 0;
					time.reset();
				}
				break;
			case _tutorial:
				if (status_number == 8) {
				}
				switch (status_number) {
					case 0:
					case 1:
					case 2:
					case 3:
					case 4:
					case 5:
					case 6:
					case 7:
					case 8:
						if (KeyZ.down()) time.start();
						if (time.sF() > 0.5) {
							status = _course;
							status_number = 0;
							time.reset();
						}
						break;
					default:
						std::cerr << "some error" << std::endl;
						System::Exit();
				}
				break;
			case _course:
				if (KeyLeft.pressed()) {
					player.move(-delta);
				}
				if (KeyRight.pressed()) {
					player.move(delta);
				}
				if (KeyUp.pressed()) player.jump(delta);
				player.update();
				break;
			case _game_finish:
				break;
			case _submit:
				break;
			case _ranking:
				break;
		}
	}
}