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
	RectF body;
	Texture body_texture;

	Vec2 speed;

	Rect last_rect;

	bool up, down, right, left;

	Stopwatch stopwatch;
	double last_jump;
	double last_inv_jump;

	int item_number;
	Array<Vec2> position_using_item;

  public:
	Player(Texture body_texture) : body(150, 430, 30, 70), speed(0, 0), up(false), down(false), right(false), left(false), item_number(0), last_inv_jump(-0.1), last_jump(-0.1), body_texture(body_texture) {
		stopwatch.start();
		last_rect = body;
	}

	void move(double a) {
		speed.x = a;
	}

	bool useItem() {
		if (item_number == 0) return false;
		position_using_item << body.pos;
		item_number--;
		return true;
	}

	Rect getRect() {
		return body;
	}

	void jump(int a) {
		if (down) {
			last_jump = stopwatch.sF();
		}
		if (stopwatch.sF() - last_jump < 0.1 && stopwatch.sF() - last_inv_jump > 0.1 && !up) speed.y -= a;
	}

	int update(Mat3x2 matrix) {
		last_rect = body;
		body.moveBy(speed.asPoint());
		speed.y += 50 * Scene::DeltaTime();
		body(body_texture).draw(Palette::White);
		if (death()) {
			System::Sleep(1s);
			up = false;
			down = false;
			left = false;
			right = false;
			speed = Vec2(0, 0);
			if (position_using_item) {
				Vec2 point = position_using_item.back();
				position_using_item.pop_back();
				body.setPos(point - Vec2(0, 50));
				return point.x;
			}
			else {
				return 0;
			}
		}
		return 1;
	}

	bool death() {
		return up && down || right && left || body.pos.y > 768;
	}

	void setIntersects(bool arg_up, bool arg_down, bool arg_right, bool arg_left) {
		up = arg_up;
		down = arg_down;
		left = arg_left;
		right = arg_right;
		if (up) {
			speed.y = std::max(speed.y, 0.0);
			last_inv_jump = stopwatch.sF();
		}
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

	double getItemNumber() {
		return item_number;
	}
};

class Course {
	Player* player;
	Vec2 course_size;

	Array<RectF> course_block;
	RectF goal;

	Array<Vec2> course_item;

	double scroll_speed;
	Mat3x2 matrix;

	double wall_right_pos, wall_left_pos;

	double clear_rate;

  public:
	Course(Player* player, Vec2 course_size, Array<RectF> course_block, RectF goal, double scroll_speed, Array<Vec2> course_item) :
	    player(player), course_size(course_size), course_block(course_block), goal(goal), scroll_speed(scroll_speed), matrix(Mat3x2::Identity()), course_item(course_item), wall_left_pos(0.0), wall_right_pos(1024.0), clear_rate(0.0) {}
	int update(double x_speed, double y_speed) {
		matrix = matrix.translated(-scroll_speed * Scene::DeltaTime(), 0);
		Transformer2D trans(matrix);
		wall_left_pos += scroll_speed * Scene::DeltaTime();
		wall_right_pos += scroll_speed * Scene::DeltaTime();
		goal.draw(Palette::White);
		bool u = false, d = false, r = false, l = false;
		Rect player_body = player->getRect();
		Rect player_last_body = player->getLastRect();
		clear_rate = Max((player_body.x - 150) / (goal.x - 150) * 100, clear_rate);
		double y = 0.0, x = 0.0;
		for (Rect it : course_block) {
			it.draw(Palette::Blue);
			if (it.intersects(player_body.scaled(1.0001))) {
				if (player_last_body.topCenter().y + 0.1 >= it.bottomCenter().y) {
					u = true;
					y = it.bottomCenter().y - player_body.topCenter().y;
				}
				if (player_last_body.bottomCenter().y - 0.1 <= it.topCenter().y) {
					d = true;
					y = it.topCenter().y - player_body.bottomCenter().y;
				}
				if (player_last_body.rightCenter().x - 0.1 <= it.leftCenter().x) {
					r = true;
					x = it.leftCenter().x - player_body.rightCenter().x;
				}
				if (player_last_body.leftCenter().x + 0.1 >= it.rightCenter().x) {
					l = true;
					x = it.rightCenter().x - player_body.leftCenter().x;
				}
			}
		}
		for (auto it = course_item.begin(); it != course_item.end();) {
			Circle it_body = Circle(*it, 10);
			if (it_body.intersects(player_body)) {
				player->getItem();
				it = course_item.erase(it);
			}
			else {
				it_body.draw(Palette::Red);
				it++;
			}
		}
		if (player_body.leftCenter().x - 0.1 <= wall_left_pos) {
			x = wall_left_pos - player_body.leftCenter().x;
			l = true;
		}
		if (player_body.rightCenter().x + 0.1 >= wall_right_pos) {
			x = wall_right_pos - player_body.rightCenter().x;
			r = true;
		}

		player->move(x_speed);
		player->jump(y_speed);
		player->setIntersects(u, d, r, l);
		player->moveY(y);
		player->moveX(x);
		int ret = player->update(matrix);
		if (0 == ret) return -1;
		if (ret != 1) setPos(ret);
		if (goal.intersects(player->getRect())) {
			System::Sleep(1s);
			return 1;
		}
		return 0;
	}

	void setPos(double pos_x) {
		pos_x = Max(1024.0, pos_x);
		matrix = Mat3x2::Translate(-pos_x + 1024, 0);
		wall_left_pos = pos_x - 1024, wall_right_pos = pos_x;
	}
	double updateMousePlus() {
		return wall_left_pos;
	}
	int getClearRate() {
		return clear_rate;
	}
	double getPlayerPosRate() {
		return (player->getPos().x - 150) / (goal.x - 150);
	}
};

void Main() {
	Status status = Status::_start;
	int status_number = (int)Status::_tutorial;

	FontManager font(80);
	HSV BackGroundColor(231, 0.63, 0.16);

	std::chrono::duration<double> clock = 2.0s;
	Stopwatch time;

	Window::Resize(1024, 768);

	double mouse_plus = 0.0;
	double use_time = -1.0;
	const Texture texture(Resource(U"game_data/player_illust.png"), TextureDesc::Mipped);
	Player player(texture);

	Array<RectF> course_block;
	Array<Vec2> course_item;
	RectF goal_rect;
	Vec2 course_size;
	{
		double x, y, w, h;
		char t;
		TextReader reader(Resource(U"game_data/course.txt"));
		if (!reader) throw Error(U"Failed to open course_data txt");
		String line;
		int cnt = 0;
		while (reader.readLine(line)) {
			if (cnt == 0) {
				x = Parse<double>(line.split(' ')[0]);
				y = Parse<double>(line.split(' ')[1]);
				course_size = Vec2(x, y);
			}
			else if (cnt == 1) {
				x = Parse<double>(line.split(' ')[0]);
				y = Parse<double>(line.split(' ')[1]);
				w = Parse<double>(line.split(' ')[2]);
				h = Parse<double>(line.split(' ')[3]);
				goal_rect = RectF(x, y, w, h);
			}
			else {
				t = line.at(0);
				line = line.substr(2);
				if (t == 'b') {
					x = Parse<double>(line.split(' ')[0]);
					y = Parse<double>(line.split(' ')[1]);
					w = Parse<double>(line.split(' ')[2]);
					h = Parse<double>(line.split(' ')[3]);
					course_block << RectF(x, y, w, h);
				}
				else {
					x = Parse<double>(line.split(' ')[0]);
					y = Parse<double>(line.split(' ')[1]);
					course_item << Vec2(x, y);
				}
			}
			cnt++;
		}
	}
	Course course(&player, course_size, course_block, goal_rect, 50.0, course_item);
	int score;
	while (System::Update()) {
		ClearPrint();
		Scene::SetBackground(BackGroundColor);
		double delta = 200 * Scene::DeltaTime();

		switch (status) {
			case Status::_start:
				font.draw_center(80, U"Escape from the cave", Scene::Center() / Vec2(1.0, 2.0), BackGroundColor, Palette::Skyblue);
				font.draw_center(40, U"---- 左クリックをしてください ----", Scene::Center() * Vec2(1.0, 1.5) + Vec2(0, 40), BackGroundColor, ColorF(1.0, 1.0, 1.0, Periodic::Square0_1(clock)));
				font.draw_center(40, U" 2019, 制作者:Osmium_1008 ", Scene::Center() * Vec2(1.0, 1.5) + Vec2(0, 100), BackGroundColor);
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
				font.draw_center(60, U" 操作説明を読みますか？", Scene::Center() / Vec2(1.0, 2.0) - Vec2(0, 70), BackGroundColor);
				font.draw_center(40, U" (当てはまる選択肢をクリックしてください)", Scene::Center() / Vec2(1.0, 2.0), BackGroundColor);
				if (font.draw_center(40, U" はい ", Scene::Center() * Vec2(0.5, 1.5), (Status)status_number == Status::_tutorial ? Color(160, 216, 239, time.isRunning() ? Periodic::Square0_1(0.2) * 255 : 255) : Color(255, 255, 255), ColorF(0, 0, 0), 1.5)
				        .mouseOver()) {
					font.draw_center(40, U"いいえ", Scene::Center() * Vec2(1.5, 1.5), (Status)status_number == Status::_course ? Color(160, 216, 239, time.isRunning() ? Periodic::Square0_1(0.2) * 255 : 255) : Color(255, 255, 255), ColorF(0, 0, 0), 1.5);
					if (!time.isRunning()) status_number = (int)Status::_tutorial;
				}
				else if (font.draw_center(
				                 40, U"いいえ", Scene::Center() * Vec2(1.5, 1.5), (Status)status_number == Status::_course ? Color(160, 216, 239, time.isRunning() ? Periodic::Square0_1(0.2) * 255 : 255) : Color(255, 255, 255), ColorF(0, 0, 0), 1.5)
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
						time.reset();
						font.draw_center(40, U"プレイヤーは棒人間です。\n\nこのプレイヤーが壁に挟まれたり\n下に落ちたりすると死にます。", Scene::Center() * Vec2(1.0, 0.6), Color(0, 0, 0, 0));
						texture.draw(Scene::Center() * Vec2(0.4, 1.2));
						if (font.draw_center(35, U" 次へ ", Scene::Center() * Vec2(1.6, 1.7), Color(160, 216, 239, 255), ColorF(0, 0, 0), 1.5).leftClicked()) {
							status_number = 1;
							System::Sleep(0.5s);
						}
						break;
					case 1:
						font.draw_center(40, U"プレイヤーはマウスカーソルに向かって進みます。\n\nまた、左クリックでジャンプをすることができます。", Scene::Center() * Vec2(1.0, 0.5), Color(0, 0, 0, 0));
						if (font.draw_center(35, U" 戻る ", Scene::Center() * Vec2(0.3, 1.7), Color(160, 216, 239, 255), ColorF(0, 0, 0), 1.5).leftClicked()) status_number = 0;
						if (font.draw_center(35, U" 次へ ", Scene::Center() * Vec2(1.6, 1.7), Color(160, 216, 239, 255), ColorF(0, 0, 0), 1.5).leftClicked()) {
							status_number = 2;
							System::Sleep(0.5s);
						}
						break;
					case 2:
						font.draw_center(40, U"赤い玉はアイテムで、\nプレイヤーが触れるとゲットできます。\nアイテムは右クリックで使用でき、使用すると\n死んだとき一度だけ使用した場所に生き返れます。", Scene::Center() * Vec2(1.0, 0.6), Color(0, 0, 0, 0));
						if (font.draw_center(35, U" 戻る ", Scene::Center() * Vec2(0.3, 1.7), Color(160, 216, 239, 255), ColorF(0, 0, 0), 1.5).leftClicked()) status_number = 1;
						if (font.draw_center(35, U"ゲームを開始する", Scene::Center() * Vec2(1.5, 1.7), Color(160, 216, 239, time.isRunning() ? Periodic::Square0_1(0.2) * 255 : 255), ColorF(0, 0, 0), 1.5).leftClicked()) time.start();
						Circle(Scene::Center() * Vec2(0.4, 1.2), 20).draw(Palette::Red);
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
				BackGroundColor.v = 0.16 + course.getPlayerPosRate() * course.getPlayerPosRate() * course.getPlayerPosRate() * 0.48;
				if (MouseR.down()) player.useItem();
				status_number = course.update(Max(Min((Cursor::Pos().x + mouse_plus - player.getPos().x) * Scene::DeltaTime() * 2, 5.0), -5.0), MouseL.pressed() * delta);
				if (status_number) {
					status = Status::_game_finish;
					time.reset();
				}
				font.draw_center(30, U"クリア率： {}%"_fmt(course.getClearRate()), Vec2(256, 735), ColorF(0, 0, 0, 0), Color(223, 84, 43));
				font.draw_center(25, U"アイテム個数： {}個"_fmt(player.getItemNumber()), Vec2(768, 735), ColorF(0, 0, 0, 0), Color(60, 214, 29));
				mouse_plus = course.updateMousePlus();
				break;
			case Status::_game_finish:
				if (status_number == -1)
					font.draw_center(80, U"ゲームオーバー...", Scene::Center() / Vec2(1.0, 2.0), BackGroundColor, Palette::Pink);
				else
					font.draw_center(80, U"ゲームクリア!!", Scene::Center() / Vec2(1.0, 2.0), BackGroundColor, Palette::Lightgreen);
				score = course.getClearRate() * 10;
				if (status_number == 1) score += (int)player.getItemNumber() * 500 + 10000;
				font.draw_center(40, U"スコア: {}"_fmt(score), Scene::Center() * Vec2(1.0, 1.3), BackGroundColor, Palette::Yellow);
				if (font.draw_center(35, U" スタート画面に戻る ", Scene::Center() * Vec2(1.0, 1.7), Color(160, 216, 239, time.isRunning() ? Periodic::Square0_1(0.2) * 255 : 255), ColorF(0, 0, 0), 1.5).leftClicked()) {
					time.start();
				}
				if (time.sF() > 0.5) {
					status = Status::_start;
					status_number = 0;
					// System::Exit();
					time.restart();
				}
				break;
			case Status::_ranking:
				break;
		}
	}
}
