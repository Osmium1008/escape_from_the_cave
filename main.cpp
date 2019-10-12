#include <Siv3D.hpp>
enum Status{
  start,tutorial,course,game_over,submit,ranking
};
void Main(){
  Status status=start,button_status=tutorial;
  const Font font30(30);
  const Font font60(60);
  Scene::SetBackground(Color(0x0f,0x13,0x29));
  while(System::Update()){
    ClearPrint();

    switch(status){
    case start:
      font60(U"Escape from the cave").draw(Arg::center=Vec2(Scene::Center().x,Scene::Center().y/2));
      font30(U"--- 左右で選択し、 Z キーを押してください ---").draw(Arg::center=Vec2(Scene::Center().x,Scene::Center().y+Scene::Center().y/2.0+60),ColorF(1.0,1.0,1.0,Periodic::Square0_1(2s)));
      if(KeyRight.down())button_status=course;
      else if(KeyLeft.down())button_status=tutorial;
      if(KeyZ.down()){
        status=button_status;
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