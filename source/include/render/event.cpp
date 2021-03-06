//Include our Class Definition
//Include Dependencies
#include "../world/world.h"
#include "../render/view.h"
#include "event.h"

void eventHandler::input(SDL_Event *e, bool &quit){
  //User input detected
  if( SDL_PollEvent( e ) != 0 ) {
    //Quit Event
    if( e->type == SDL_QUIT ) { quit = true; }
    //Key Down Top-Event
    else if( e->type == SDL_KEYDOWN ) {
      if(e->key.keysym.sym == SDLK_UP  && rotate.empty()){
        rotate.push_front(e);
      }
      else if(e->key.keysym.sym == SDLK_DOWN  && rotate.empty()){
        rotate.push_front(e);
      }
      else if(e->key.keysym.sym == SDLK_F11){
        //Add fullscreen mode
        fullscreen = true;
      }
      else{
        inputs.push_front(e);
      }
    }
    else if( e->type == SDL_MOUSEWHEEL){
      scroll.push_front(e);
    }
    else if( e->type == SDL_MOUSEBUTTONDOWN){
      mouse = e;
      click = true;
    }
    else if( e->type == SDL_MOUSEBUTTONUP){
      mouse = e;
      click = true;
    }
    else if( e->type == SDL_MOUSEMOTION){
      mouse = e;
      move = true;
    }
    else if( e->type == SDL_WINDOWEVENT){
      windowevent = e;
      _window = true;
    }
  }
}

void eventHandler::update(World &world, View &view, bool &paused){
  bool updateCamera = false;
  if(_window && windowevent->window.event == SDL_WINDOWEVENT_RESIZED ){
    //Change the Screen Width and Height
    view.SCREEN_WIDTH = windowevent->window.data1;
    view.SCREEN_HEIGHT = windowevent->window.data2;
    view.projection = glm::ortho(-(float)view.SCREEN_WIDTH*view.zoom, (float)view.SCREEN_WIDTH*view.zoom, -(float)view.SCREEN_HEIGHT*view.zoom, (float)view.SCREEN_HEIGHT*view.zoom, -100.0f, 100.0f);
    _window = false;
  }
  if(fullscreen){
    //Toggle fullscreen mode
    if(!view.fullscreen){
      SDL_SetWindowFullscreen(view.gWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
      view.fullscreen = true;
    }
    else{
      SDL_SetWindowFullscreen(view.gWindow, 0);
      view.fullscreen = false;
    }
    fullscreen = false;
  }

  //Check for rotation
  if(!inputs.empty()){
    //Handle the Player Move
    if(inputs.front()->key.keysym.sym == SDLK_w){
      handlePlayerMove(world, view, 0);
    }
    else if(inputs.front()->key.keysym.sym == SDLK_a){
      handlePlayerMove(world, view, 1);
    }
    else if(inputs.front()->key.keysym.sym == SDLK_s){
      handlePlayerMove(world, view, 2);
    }
    else if(inputs.front()->key.keysym.sym == SDLK_d){
      handlePlayerMove(world, view, 3);
    }
    else if(inputs.front()->key.keysym.sym == SDLK_p){
      paused = !paused;
    }
    else if(inputs.front()->key.keysym.sym == SDLK_SPACE){
      handlePlayerMove(world, view, 4);
    }
    else if(inputs.front()->key.keysym.sym == SDLK_c){
      handlePlayerMove(world, view, 5);
    }
    else if(inputs.front()->key.keysym.sym == SDLK_ESCAPE){
      view.showmenu = !view.showmenu; //Toggle Menu Visibility
    }
    //Remove the command
    inputs.pop_back();
  }

  if(!rotate.empty()){
    //We want to perform an event now..
    if(rotate.front()->key.keysym.sym == SDLK_UP) view.cameraPos += glm::vec3(0, 10, 0);
    else if(rotate.back()->key.keysym.sym == SDLK_DOWN) view.cameraPos -= glm::vec3(0, 10, 0);
    rotate.pop_back();
    updateCamera = true;
  }

  if(!scroll.empty()){
    //Scroll Away
    if(scroll.front()->wheel.y > 0.99 && view.zoom <= 0.3){
      //For Perspective Camera
    //  view.cameraPos += glm::vec3(10, 0, 10);
      updateCamera = true;

      //For Orthogonal Camera
      view.zoom += view.zoomInc;
      view.projection = glm::ortho(-(float)view.SCREEN_WIDTH*view.zoom, (float)view.SCREEN_WIDTH*view.zoom, -(float)view.SCREEN_HEIGHT*view.zoom, (float)view.SCREEN_HEIGHT*view.zoom, -800.0f, 500.0f);

      scroll.pop_back();
    }
    //Scroll Closer
    else if(scroll.back()->wheel.y < -0.99 && view.zoom > 0.005){
      //Perspective Camera
    //  view.cameraPos -= glm::vec3(10, 0, 10);
      updateCamera = true;

      //Orthogonal Camera
      view.zoom -= view.zoomInc;
      view.projection = glm::ortho(-(float)view.SCREEN_WIDTH*view.zoom, (float)view.SCREEN_WIDTH*view.zoom, -(float)view.SCREEN_HEIGHT*view.zoom, (float)view.SCREEN_HEIGHT*view.zoom, -800.0f, 500.0f);

      scroll.pop_back();
    }
    else if(scroll.back()->wheel.x < -0.8){
      glm::vec3 axis(0.0f, 1.0f, 0.0f);
      view.rotation += 1.5f;
      view.camera = glm::rotate(view.camera, glm::radians(1.5f), axis);
      //view.sprite.model = glm::rotate(view.sprite.model, glm::radians(-1.5f), axis);
      scroll.pop_back();
    }
    else if(scroll.back()->wheel.x > 0.8){
      glm::vec3 axis(0.0f, 1.0f, 0.0f);
      view.rotation -= 1.5f;
      view.camera = glm::rotate(view.camera, glm::radians(-1.5f), axis);
      //view.sprite.model = glm::rotate(view.sprite.model, glm::radians(1.5f), axis);
      scroll.pop_back();
    }
    if(view.rotation < 0.0){
      view.rotation = 360.0 + view.rotation;
    }
    if(view.rotation > 360.0){
      view.rotation = view.rotation - 360.0;
    }
    updateCamera = true;
  }
  //Fix the Camera
  if(updateCamera)
    view.camera = glm::rotate(glm::lookAt(view.cameraPos, view.lookPos, glm::vec3(0,1,0)), glm::radians(view.rotation), glm::vec3(0,1,0));
}

void eventHandler::handlePlayerMove(World &world, View &view, int a){
  //Movement Vector
  glm::vec3 m;

  //Make sure we rotate correctly!
  if(a < 4){
    //Rotational Position
    int g = floor(abs(view.rotation/90));

    //Rotate the Guy
    if(g%2) a = (a+g+2)%4;
    else a = (a+g)%4;
  }

  //Get the type of movement
  switch(a){
    //W
    case 0:
      m = glm::vec3(0,0,-1);
      break;
    //A
    case 1:
      m = glm::vec3(-1,0,0);
      break;
    //S
    case 2:
      m = glm::vec3(0,0,1);
      break;
    //D
    case 3:
      m = glm::vec3(1,0,0);
      break;
    case 4:
      m = glm::vec3(0,1,0);
      break;
    case 5:
      m = glm::vec3(0,-1,0);
      break;
  }
  view.model.translate(-m);
}
