#ifndef buzzer_included
#define buzzer_included

#define wall_col_note 3370
#define player_col_note 4500

void buzzer_init();
void buzzer_set_period(short cycles);

void play_note();

#endif
