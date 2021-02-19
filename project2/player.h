#ifndef player_included
#define player_included

typedef struct player_s{
  void (*getBounds)(const struct player_s *player, const Vec2 *centerPos, Region *bounds);
  int (*check)(const struct player_s *shape, const Vec2 *centerPos, const Vec2 *pixel);
} Player;

void playerGetBounds(const Player *player, const Vec2 *center, Region *bounds);

int playerCheck(const Player *player, const Vec2 *center, const Vec2 *pixel);

//void updateDirection(Layer *layer);

#endif
