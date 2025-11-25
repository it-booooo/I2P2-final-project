#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include "level_switch.h"
#include "monster_factory.h"
#include "allegro5/allegro_primitives.h"
#include "susu.h"
#include "../global.h"

static LevelState state = LVL_RUNNING;
static double     trans_timer = 0.0;             
static ALLEGRO_FONT *level_font = NULL;
static ALLEGRO_FONT *story_font = NULL;
static ALLEGRO_BITMAP *intro_img = NULL; 
static double time_space = 10.0;
static bool level_up=0;

bool is_over()
{
    return level_no >= 5 && state==LVL_TRANSITION;
}

void Level_switch_Init(void)
{
    state = LVL_RUNNING;
    trans_timer = 0.0;
    level_no = 0; 
    time_space = 8.0;
    if (!level_font) level_font = al_load_ttf_font("assets/font/pirulen.ttf", 64, 0);
    if (!story_font) story_font = al_load_ttf_font("assets/font/pirulen.ttf", 32, 0);
    if (!intro_img)intro_img = al_load_bitmap("assets/image/intro.png");          
}

LevelState Level_switch_GetState(void) { return state; }

void Level_switch_Update(Scene *scene, double dt)
{
    switch (state) {
    case LVL_RUNNING:
        /* 1) 先讓 MonsterFactory 正常運作 */
        MF_Update(scene, dt);

        /* 2) 場上清光且下一波是 buffer → 進 TRANSITION */
        if (MF_AliveMonsterCount(scene) == 0 && MF_NextWaveIsBuffer()) {
            state = LVL_TRANSITION;
            trans_timer = 0.0;
        }
        break;

    case LVL_TRANSITION:
        trans_timer += dt;
        
        if(level_no==0)time_space=10.0;
        else time_space=8.0;
        if (trans_timer > time_space) {      /* 黑幕 5 秒 */
            MF_SkipBufferWave();       /* 跳過 buffer wave */
            ++level_no;
            level_up=0;
            state = LVL_RUNNING;
        }
        break;

    case LVL_FINISHED:
        /* 可擴充結束畫面 */
        break;
    }
}

void Level_switch_DrawOverlay(void)
{
    if (state != LVL_TRANSITION) return;
    
    if(level_up == 0)
    {
        susu *chara = ((susu *)(get_susu()->pDerivedObj));
        chara->base.hp = chara->base.full_hp;
        chara->damage+=10;
        level_up =1;
    }
    

    if(level_no<5)al_draw_filled_rectangle(0,0,WIDTH,HEIGHT, al_map_rgb(0,0,0));
    if(time_space - trans_timer > 1)
    {
        if (story_font) 
        {
            char story[1000];
            switch (level_no) 
            {
                case 0:
                    if(trans_timer < 5.0)
                    {
                        sprintf(story, "One morning, Susu woke up in his dorm\nand found himself transported to\nan alternate NTHU overrun by Italian monsters.\n Determined to finish his final programming project,\nhe bravely picked up his sword and set off on his quest.");
                        //al_draw_text(level_font, al_map_rgb(255,255,255),WIDTH/2, 100, ALLEGRO_ALIGN_CENTRE, buf);
                        al_draw_multiline_text(story_font,al_map_rgb(255,255,255),WIDTH/2, HEIGHT/2-200,2000,36,ALLEGRO_ALIGN_CENTRE,story);
                    }
                    else
                    {
                        al_draw_bitmap(intro_img, 0, 100, 0);
                    }
                    break;
                case 1:
                    sprintf(story, "Before stepping onto the great lawn,\nSusu had heard of banana apes\nand tree monsters—something felt off.\nBananini leapt out laughing wildly,\nwhile Patapim rose like an ancient tree.\nSusu swung his sword while whispering while loop logic.");
                    al_draw_multiline_text(story_font,al_map_rgb(255,255,255),WIDTH/2, HEIGHT/2-200,2000,36,ALLEGRO_ALIGN_CENTRE,story);
                    break;
                case 2:
                    sprintf(story, "As Susu approached Chenggong Lake,\nthe still water felt unnaturally quiet—like a trap.\nTrippi emerged, firing bubble arrows;\n Tralala burst out in Nike shoes.\nSusu rolled to dodge, like debugging a fatal error.");
                    al_draw_multiline_text(story_font,al_map_rgb(255,255,255),WIDTH/2, HEIGHT/2-200,2000,36,ALLEGRO_ALIGN_CENTRE,story);
                    break;
                case 3:
                    sprintf(story, "Before entering the classroom,\nSusu thought he could finally code in peace\nbut even VS Code wasn’t safe.\nCappuccino flickered in and out of invincibility,\nlaughing maniacally. Crocodilo launched tracking missiles.\nSusu coded functions while dodging bugs and reptiles.");
                    al_draw_multiline_text(story_font,al_map_rgb(255,255,255),WIDTH/2, HEIGHT/2-200,2000,36,ALLEGRO_ALIGN_CENTRE,story);
                    break;
                case 4:
                    sprintf(story, "Just as Susu thought it was over and moved to save his project,\nthe screen flickered—something familiar\nyet ominous approached.\nA colossal Tungtungtung crawled out,\nshrouded in debugging smoke.\nCan Susu withstand the final onslaught?");
                    al_draw_multiline_text(story_font,al_map_rgb(255,255,255),WIDTH/2, HEIGHT/2-200,2000,36,ALLEGRO_ALIGN_CENTRE,story);
                    break;
            }
        }
    }
    else
    {
        if (level_font && level_no<5) {
            al_draw_filled_rectangle(0,0,WIDTH,HEIGHT, al_map_rgb(0,0,0));
            char buf[32];
            sprintf(buf, "LEVEL %d", level_no);
            al_draw_text(level_font, al_map_rgb(255,255,255),WIDTH/2, HEIGHT/2-100, ALLEGRO_ALIGN_CENTRE, buf);
        }
    }
}

void Level_switch_Destroy(void)
{
    if (level_font) {
        al_destroy_font(level_font);
        level_font = NULL;
    }
    if (story_font) {
        al_destroy_font(story_font);
        story_font = NULL;
    }
    
    if (intro_img) {
        al_destroy_bitmap(intro_img);
        intro_img = NULL;
    }
}