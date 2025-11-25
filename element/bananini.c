/* Bananini ― 會射 banana.png */
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_image.h>
#include "bananini.h"
#include "susu.h"
#include "atk.h"
#include "../scene/sceneManager.h"
#include "../shapes/Rectangle.h"
#include "../global.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "../scene/gamescene.h"

/* 參數同上 */
#define CHASE_SPEED     2.0f
#define ARRIVE_EPSILON 400.0f
#define COOLDOWN_FRAMES 180
#define BULLET_DAMAGE   40
#define BULLET_SPEED    3.0f
#define SHOT_RANGE      500.0f
static void _banana_update_position(Elements *self,int dx,int dy);

/* 建構 */
Elements *New_bananini(int label)
{
    bananini *pD=malloc(sizeof(bananini));
    Elements *pE=New_Elements(label);
    const char *state_s[3]={"stop","move","atk"};
    for(int i=0;i<3;++i){
        char buf[64];
        sprintf(buf,"assets/image/ChimpanziniBananini_%s.png",state_s[i]);
        pD->img[i]=al_load_bitmap(buf);
    }
    pD->width =al_get_bitmap_width (pD->img[0]);
    pD->height=al_get_bitmap_height(pD->img[0]);

    Elements *plE=get_susu();
    susu *pl=plE?(susu*)plE->pDerivedObj:NULL;
    do{
        pD->x=rand()%(WIDTH -pD->width );
        pD->y=rand()%(HEIGHT-pD->height);
    }while(pl&&fabs(pD->x-pl->x)<ARRIVE_EPSILON&&
                fabs(pD->y-pl->y)<ARRIVE_EPSILON);

    pD->base.hp=60; pD->base.side=1;
    pD->base.hitbox=New_Rectangle(pD->x,pD->y,
                                  pD->x+pD->width ,
                                  pD->y+pD->height);

    pD->dir=false; pD->state=STOP; pD->cooldown=0;

    pE->pDerivedObj=pD;
    pE->Draw   =bananini_draw;
    pE->Update =bananini_update;
    pE->Interact=bananini_interact;
    pE->Destroy =bananini_destory;
    return pE;
}

/* Update：射 banana.png */
void bananini_update(Elements *self)
{
    bananini *ch=self->pDerivedObj;
    if(ch->cooldown>0) ch->cooldown--;
    Elements *plE=get_susu(); if(!plE) return;
    susu *pl=plE->pDerivedObj;

    int cx=ch->x+ch->width/2, cy=ch->y+ch->height/2;
    int tx=pl->x+pl->width/2, ty=pl->y+pl->height/2;
    int dx=tx-cx, dy=ty-cy;
    float dist=sqrtf((float)dx*dx+(float)dy*dy);

    if(dist>SHOT_RANGE){
        float vx=CHASE_SPEED*dx/dist, vy=CHASE_SPEED*dy/dist;
        _banana_update_position(self,(int)vx,(int)vy);
        ch->dir=(dx>=0); ch->state=MOVE;
    }else ch->state=STOP;

    if(ch->state==STOP&&ch->cooldown==0){
        if(dist<1.0f) dist=1.0f;
        float fx=BULLET_SPEED*dx/dist, fy=BULLET_SPEED*dy/dist;
        int vx=(int)roundf(fx), vy=(int)roundf(fy);
        if(vx==0) vx=(dx>0)?1:-1;
        if(vy==0) vy=(dy>0)?1:-1;

        Elements *proj=New_Atk(Atk_L,cx-20,cy-20,
                               (float)vx,(float)vy,
                               BULLET_DAMAGE,1);
        if(proj){
            Atk_set_image(proj,"assets/image/Banana.png");
            _Register_elements(scene,proj);
        }
        ch->cooldown=COOLDOWN_FRAMES;
    }
}

void bananini_draw(Elements *self)
{
    bananini *ch=self->pDerivedObj;
    ALLEGRO_BITMAP *bmp=ch->img[ch->state];
    if(!bmp) return;
    al_draw_bitmap(bmp,ch->x,ch->y,ch->dir?ALLEGRO_FLIP_HORIZONTAL:0);
}
void bananini_interact(Elements *self){}
void bananini_destory(Elements *self)
{
    bananini *ch=self->pDerivedObj;
    for(int i=0;i<3;++i) if(ch->img[i]) al_destroy_bitmap(ch->img[i]);
    free(ch->base.hitbox); free(ch); free(self);
}
static void _banana_update_position(Elements *self,int dx,int dy)
{
    bananini *ch=self->pDerivedObj;
    ch->x+=dx; ch->y+=dy;
    if(ch->x<0) ch->x=0;
    if(ch->y<0) ch->y=0;
    if(ch->x>WIDTH -ch->width ) ch->x=WIDTH -ch->width ;
    if(ch->y>HEIGHT-ch->height) ch->y=HEIGHT-ch->height;
    Shape *hb=ch->base.hitbox;
    hb->update_center_x(hb,dx);
    hb->update_center_y(hb,dy);
}
