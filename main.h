#ifndef MAIN_H
#define MAIN_H

int POSX = 500;
int POSY = 500;
int WIDTH = 500;
int HEIGHT = 500;
int BORDER = 15;
char* title = "LunarWM";

int main(int argc, char* argv[]);
void grabKey(char *key, unsigned int mod);
void OnCreateNotify(const XCreateWindowEvent& e);
void OnDestroyNotify(const XDestroyWindowEvent& e);
void OnExpose(const XExposeEvent& e);
void OnKeyPress(const XKeyEvent& e);
void OnReperentNotify(const XReparentEvent& e);

#endif // MAIN_H