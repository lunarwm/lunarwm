#ifndef MAIN_H
#define MAIN_H

#define POSX 200
#define POSY 200
#define WIDTH 800
#define HEIGHT 600
#define BORDER 4

#define CHECK(X) if (!(X)) { fprintf(stderr, "%s:%d: %s failed\n", __FILE__, __LINE__, #X); }

#endif