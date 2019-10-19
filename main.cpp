#include <Siv3D.hpp>

enum class Status { _start, _select, _tutorial, _course, _game_finish, _submit, _ranking };

class FontManager {
	Array<Font> fontset;

  public:
	FontManager(int max_size) : fontset(max_size, Font(0)){};
	Rect draw_center(int font_size, String text, Vec2 pos, HSV rect_color, ColorF color = ColorF(1.0, 1.0, 1.0), double rect_size = 1.0) {
		if (fontset.at(font_size - 1) == Font(0)) fontset[font_size - 1] = Font(font_size);
		fontset.at(font_size - 1)(text).region(Arg::center = pos).scaled(rect_size).draw(rect_color);
		return fontset.at(font_size - 1)(text).draw(Arg::center = pos, color).scaled(rect_size);
	}
};

class Player {
	Rect body;

	Vec2 speed;

	Rect last_rect;

	bool up, down, right, left;

	Stopwatch stopwatch;
	double last_jump;

	int item_number;
	Array<Vec2> position_using_item;

  public:
	Player() : body(150, 430, 30, 70), speed(0, 0), up(false), down(false), right(false), left(false), item_number(0) {
		stopwatch.start();
		last_rect = body;
	}

	void move(double a) {
		speed.x = a;
	}

	bool use_item() {
		if (item_number == 0) return false;
		position_using_item << body.pos;
		return true;
	}

	Rect getRect() {
		return body;
	}

	void jump(int a) {
		if (down) {
			last_jump = stopwatch.sF();
		}
		if (stopwatch.sF() - last_jump < 0.1 && !up) speed.y -= a;
	}

	bool update(Mat3x2 matrix) {
		last_rect = body;
		body.moveBy(speed.asPoint());
		speed.y += 50 * Scene::DeltaTime();
		body.draw(Palette::White);
		if (death()) {
			if (position_using_item) {
				Vec2 point = position_using_item.back();
				position_using_item.pop_back();
				body.setPos(point.Up(30).asPoint());
			}
			else {
				return false;
			}
		}
		return true;
	}

	bool death() {
		return up && down || right && left || body.pos.y > 600;
	}

	void setIntersects(bool arg_up, bool arg_down, bool arg_right, bool arg_left) {
		up = arg_up;
		down = arg_down;
		left = arg_left;
		right = arg_right;
		if (up) speed.y = std::max(speed.y, 0.0);
		if (down) speed.y = std::min(speed.y, 0.0);
		if (right) speed.x = std::min(speed.x, 0.0);
		if (left) speed.x = std::max(speed.x, 0.0);
	}
	Rect getLastRect() {
		return last_rect;
	}

	Vec2 getPos() {
		return body.center();
	}

	void moveY(double y) {
		body.moveBy(0, y);
	}

	void moveX(double x) {
		body.moveBy(x, 0);
	}

	void getItem() {
		++item_number;
	}
};

class Course {
	Player* player;
	Vec2 course_size;

	Array<Rect> course_block;
	Rect goal;

	double scroll_speed;
	Mat3x2 matrix;

  public:
	Course(Player* player, Vec2 course_size, Array<Rect> course_block, Rect goal, double scroll_speed) : player(player), course_size(course_size), course_block(course_block), goal(goal), scroll_speed(scroll_speed), matrix(Mat3x2::Identity()) {}
	int update(double x_speed, double y_speed) {
		matrix = matrix.translated(-scroll_speed * Scene::DeltaTime(), 0);
		Transformer2D trans(matrix);
		goal.draw(Palette::White);
		bool u = false, d = false, r = false, l = false;
		Rect player_body = player->getRect();
		Rect player_last_body = player->getLastRect();
		double y = 0.0;
		for (Rect it : course_block) {
			it.draw(Palette::Blue);
			if (it.intersects(player_body)) {
				if (player_last_body.topCenter().y >= it.bottomCenter().y) {
					u = true;
					y = it.bottomCenter().y - player_body.topCenter().y;
				}
				if (player_last_body.bottomCenter().y <= it.topCenter().y || player_body.bottomCenter().y == player_last_body.bottomCenter().y) {
					d = true;
					y = it.topCenter().y - player_body.bottomCenter().y;
				}
				if (player_last_body.rightCenter().x <= it.leftCenter().x) r = true;
				if (player_last_body.leftCenter().x >= it.rightCenter().x) l = true;
			}
		}
		player->move(x_speed);
		player->jump(y_speed);
		player->setIntersects(u, d, r, l);
		player->moveY(y);
		Print << u << d << r << l;
		if (!(player->update(matrix))) return -1;
		if (goal.intersects(player->getRect())) return 1;
		return 0;
	}
};

void Main() {
	Status status = Status::_start;
	int status_number = (int)Status::_tutorial;

	FontManager font(60);
	HSV BackGroundColor(231, 0.63, 0.16);

	std::chrono::duration<double> clock = 2.0s;
	Stopwatch time;

	Window::Resize(800, 600);

	double mouse_plus = 0.0;
	double use_time = -1.0;
	Player player;

	Array<Rect> course_block;
	course_block << Rect(0, 550, 2400, 50) << Rect(0, 0, 2400, 50);
	Course course(&player, Vec2(2400, 600), course_block, Rect(2200, 400, 30, 100), 50.0);

	while (System::Update()) {
		ClearPrint();
		Scene::SetBackground(BackGroundColor);
		double delta = 200 * Scene::DeltaTime();

		switch (status) {
			case Status::_start:
				font.draw_center(60, U"Escape from the cave", Scene::Center() / Vec2(1.0, 2.0), BackGroundColor);
				font.draw_center(30, U"---- 左クリックをしてください ----", Scene::Center() * Vec2(1.0, 1.5) + Vec2(0, 40), BackGroundColor, ColorF(1.0, 1.0, 1.0, Periodic::Square0_1(clock)));
				font.draw_center(30, U" 2019, 制作者:Osmium_1008 ", Scene::Center() * Vec2(1.0, 1.5) + Vec2(0, 90), BackGroundColor);
				if (MouseL.down()) {
					clock = 0.2s;
					time.start();
				}
				if (time.sF() > 0.5) {
					status = Status::_select;
					time.reset();
				}
				break;
			case Status::_select:
				font.draw_center(45, U" このゲームを遊ぶのは初めてですか？", Scene::Center() / Vec2(1.0, 2.0) - Vec2(0, 70), BackGroundColor);
				font.draw_center(30, U" 当てはまる選択肢をクリックしてください", Scene::Center() / Vec2(1.0, 2.0), BackGroundColor);
				if (font.draw_center(30, U" はい ", Scene::Center() * Vec2(0.5, 1.5), (Status)status_number == Status::_tutorial ? Color(160, 216, 239, time.isRunning() ? Periodic::Square0_1(0.2) * 255 : 255) : Color(255, 255, 255), ColorF(0, 0, 0), 1.5)
				        .mouseOver()) {
					font.draw_center(30, U"いいえ", Scene::Center() * Vec2(1.5, 1.5), (Status)status_number == Status::_course ? Color(160, 216, 239, time.isRunning() ? Periodic::Square0_1(0.2) * 255 : 255) : Color(255, 255, 255), ColorF(0, 0, 0), 1.5);
					if (!time.isRunning()) status_number = (int)Status::_tutorial;
				}
				else if (font.draw_center(
				                 30, U"いいえ", Scene::Center() * Vec2(1.5, 1.5), (Status)status_number == Status::_course ? Color(160, 216, 239, time.isRunning() ? Periodic::Square0_1(0.2) * 255 : 255) : Color(255, 255, 255), ColorF(0, 0, 0), 1.5)
				             .mouseOver()) {
					if (!time.isRunning()) status_number = (int)Status::_course;
				}
				else if (!time.isRunning())
					status_number = -1;
				if (MouseL.down() && status_number != -1) {
					clock = 0.2s;
					time.start();
				}
				if (time.sF() > 0.5) {
					status = (Status)status_number;
					status_number = 0;
					time.restart();
				}
				break;
			case Status::_tutorial:
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
						if (MouseL.down()) time.start();
						if (time.sF() > 0.5) {
							status = Status::_course;
							status_number = 0;
							time.restart();
						}
						break;
					default:
						std::cerr << "some error" << std::endl;
						System::Exit();
				}
				break;
			case Status::_course:
				if (MouseR.down())
					if (!player.use_item()) use_time = time.sF();
				if (time.sF() - use_time < 0.5) font.draw_center(15, U"アイテムを持っていません", Vec2(500, 100), BackGroundColor, ColorF(1.0, 1.0, 1.0, Periodic::Square0_1(0.2s)));

				if (status_number = course.update((Cursor::Pos().x + mouse_plus - player.getPos().x) * Scene::DeltaTime() * 2, MouseL.pressed() * delta)) {
					status = Status::_game_finish;
				}
				mouse_plus += 50.0 * Scene::DeltaTime();
				break;
			case Status::_game_finish:
				if (status_number == -1)
					font.draw_center(60, U"ゲームオーバー…", Scene::Center() / Vec2(1.0, 2.0), BackGroundColor, Palette::Lightpink);
				else
					font.draw_center(60, U"ゲームクリア!!", Scene::Center() / Vec2(1.0, 2.0), BackGroundColor, Palette::Lightpink);
				break;
		}
	}
}