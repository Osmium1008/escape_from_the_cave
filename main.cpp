#include <Siv3D.hpp>
enum Status{
  start,select,tutorial,course,game_over,submit,ranking
};

class FontManager {
	Array<Font> fontset;
public:
	FontManager(int max_size) :fontset(max_size,Font(0)) {};
	Rect draw_center(int font_size, String text, Vec2 pos,ColorF color=ColorF(1.0,1.0,1.0),double rect_size=1.0,Color rect_color= Color(0x0f, 0x13, 0x29)) {
		if (fontset.at(font_size - 1) == Font(0))fontset[font_size - 1] = Font(font_size);
		fontset.at(font_size - 1)(text).region(Arg::center = pos).scaled(rect_size).draw(rect_color);
		return fontset.at(font_size - 1)(text).draw(Arg::center = pos,color);
	}
};
void Main(){
  Status status=start,button_status=tutorial;
  
  FontManager font(60); 
  Scene::SetBackground(Color(0x0f,0x13,0x29));

  std::chrono::duration<double> clock = 2.0s;
  Stopwatch time;
  

  while(System::Update()){
    ClearPrint();

    switch(status){
    case start:
	  font.draw_center(60,U"Escape from the cave", Scene::Center() / Vec2(1.0, 2.0));
      font.draw_center(30,U"---- Z キーを押してください ----", Scene::Center() * Vec2(1.0, 1.5)+Vec2(0,40),ColorF(1.0,1.0,1.0,Periodic::Square0_1(clock)));
	  font.draw_center(30,U" 2019, 制作者:Osmium_1008 ", Scene::Center() * Vec2(1.0, 1.5) + Vec2(0, 90));
      if(KeyZ.down()){
		  clock = 0.2s;
		  time.start();
      }
	  if (time.sF() > 0.5) {
		  status = select;
		  time.reset();
	  }
      break;
	case select:
		font.draw_center(45, U" このゲームを遊ぶのは初めてですか？", Scene::Center() / Vec2(1.0,2.0) - Vec2(0, 70));
		font.draw_center(30, U" 左右キー:選択, Zキー:決定", Scene::Center() / Vec2(1.0, 2.0));
		font.draw_center(30, U" はい ", Scene::Center() * Vec2(0.5, 1.5), ColorF(0, 0, 0), 1.5, button_status == tutorial ? Color(160, 216, 239, time.isRunning() ? Periodic::Square0_1(0.2)*255: 255) : Color(255, 255, 255));
		font.draw_center(30, U"いいえ", Scene::Center() * Vec2(1.5, 1.5), ColorF(0, 0,0),1.5, button_status == course ? Color( 160,  216, 239, time.isRunning() ? Periodic::Square0_1(0.2) * 255 : 255) : Color(255, 255, 255));
		if (KeyRight.down())button_status = course;
		if (KeyLeft.down())button_status = tutorial;
		if (KeyZ.down()) {
			clock = 0.2s;
			time.start();
		}
		if (time.sF() > 0.5) {
			status = button_status;
			time.reset();
		}
		break;
    case tutorial:
      std::cout<<2<<std::endl;
      break;
    case course:
      break;
    case game_over:
      break;
    case submit:
      break;
    case ranking:
      break;
    }
  }
}