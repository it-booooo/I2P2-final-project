#include <allegro5/allegro.h>
#include "monster_factory.h"
#include "../element/tungtungtung.h"
#include "../element/susu.h"
#include "../element/ball.h"
#include "../element/trippi_troppi.h"
#include "../element/element.h"
#include "../element/capuccino.h"
#include "../element/bananini.h"
#include "../element/patapim.h"
#include "../element/tralala.h"
#include "../element/crocodilo.h"
#include "../element/bigtung.h"


/* ---------------------------------------------------------
   波次腳本用的資料結構
   --------------------------------------------------------- */
typedef struct {
    MonsterType type;   /* 怪物種類 */
    int         count;  /* 數量     */
    bool        is_buffer;  /* 切關緩衝 */
} SpawnEntry;

typedef struct {
    const SpawnEntry *entries; /* 指向本波的 SpawnEntry 陣列 */
    int               entry_num;
    bool              is_buffer;
} Wave;

/* ---------------------------------------------------------
   這是你的波次腳本 (只改這裡就可以增刪波次)
   --------------------------------------------------------- */
static const SpawnEntry wave000_entries[] = { //用來跳過的
};
static const SpawnEntry wave00_entries[] = {
};
/* Wave 0 : 3 × tung */
static const SpawnEntry wave0_entries[] = {
    { MON_TUNGTUNGTUNG, 3},
};
/* Wave 0.5 : 1 × tung */
static const SpawnEntry wave05_entries[] = {
    { MON_TUNGTUNGTUNG, 1 },
};
/* Wave 1 : 5 × bananini */
static const SpawnEntry wave1_entries[] = {
   {MON_BANANINI, 5},
};
/* Wave 2 : 5 × tung , 3 × bananini */
static const SpawnEntry wave2_entries[] = {
    { MON_TUNGTUNGTUNG, 5 },
    {MON_BANANINI, 3},
};

/* Wave 3 :patapim x 1, 2 × tung , 4 × bananini */
static const SpawnEntry wave3_entries[] = {
    { MON_BANANINI, 4 },
    { MON_PATAPIM, 5 },
};

/* Wave 4 :level one complate */
static const SpawnEntry wave4_entries[] = {
    { MON_TUNGTUNGTUNG, 1 },
};

/* Wave 5 :trippi_troppi x 4, 2 × tung */
static const SpawnEntry wave5_entries[] = {
    { MON_TUNGTUNGTUNG, 2 },
    { MON_TRIPPI_TROPPI, 4 },
};

/* Wave 6 :trippi_troppi x 5, 3 × bananini */
static const SpawnEntry wave6_entries[] = {
    { MON_BANANINI, 3 },
    { MON_TRIPPI_TROPPI, 5 },
};

/* Wave 7 :trippi_troppi x 3, bananini x 3 , tralala x 1*/
static const SpawnEntry wave7_entries[] = {
    { MON_TRALALA, 1 },
    { MON_TRIPPI_TROPPI, 3 },
    { MON_BANANINI, 3},
};

/* Wave 8 : level two complete*/
static const SpawnEntry wave8_entries[] = {
    { MON_TUNGTUNGTUNG, 1 },
};

/* Wave 9 : assassino x 5*/
static const SpawnEntry wave9_entries[] = {
    { MON_CAPUCCINO, 5 },
};

/* Wave 10 :assassino x 3, bananini x 3*/
static const SpawnEntry wave10_entries[] = {
    { MON_CAPUCCINO, 3 },
    { MON_BANANINI, 3},
};

/* Wave 11 :crocodilo x 1, assassino x 3 */
static const SpawnEntry wave11_entries[] = {
    { MON_CROCODILO, 1 },
    { MON_CAPUCCINO, 3 },
};

/* Wave 12 : level three complete*/
static const SpawnEntry wave12_entries[] = {
    { MON_TUNGTUNGTUNG, 1 },
};

/* Wave 13 :bigtung x 1, tungtungtung x 10*/
static const SpawnEntry wave13_entries[] = {
    { MON_BIGTUNG, 1 },
    { MON_TUNGTUNGTUNG, 10 },
};
static const SpawnEntry wave14_entries[] = {
};
/* 把每波統整在同一個陣列，方便用 wave_idx 存取 */
static const Wave g_waves[] = {
    { wave000_entries, sizeof(wave000_entries)/sizeof(wave000_entries[0]) ,false},
    { wave00_entries, sizeof(wave00_entries)/sizeof(wave00_entries[0]) ,true},
    { wave0_entries, sizeof(wave0_entries)/sizeof(wave0_entries[0]) ,false},
    { wave05_entries, sizeof(wave05_entries)/sizeof(wave05_entries[0]) ,true},
    { wave1_entries, sizeof(wave1_entries)/sizeof(wave1_entries[0]) ,false},
    { wave2_entries, sizeof(wave2_entries)/sizeof(wave2_entries[0]) ,false},
    { wave3_entries, sizeof(wave3_entries)/sizeof(wave3_entries[0]) ,false},
    { wave4_entries, sizeof(wave4_entries)/sizeof(wave4_entries[0]) ,true},
    { wave5_entries, sizeof(wave5_entries)/sizeof(wave5_entries[0]) ,false},
    { wave6_entries, sizeof(wave6_entries)/sizeof(wave6_entries[0]) ,false},
    { wave7_entries, sizeof(wave7_entries)/sizeof(wave7_entries[0]) ,false},
    { wave8_entries, sizeof(wave8_entries)/sizeof(wave8_entries[0]) ,true},
    { wave9_entries, sizeof(wave9_entries)/sizeof(wave9_entries[0]) ,false},
    { wave10_entries, sizeof(wave10_entries)/sizeof(wave10_entries[0]) ,false},
    { wave11_entries, sizeof(wave11_entries)/sizeof(wave11_entries[0]) ,false},
    { wave12_entries, sizeof(wave12_entries)/sizeof(wave12_entries[0]) ,true},
    { wave13_entries, sizeof(wave13_entries)/sizeof(wave13_entries[0]) ,false},
    { wave14_entries, sizeof(wave14_entries)/sizeof(wave14_entries[0]) ,true},
};
static const int g_wave_cnt = sizeof(g_waves)/sizeof(g_waves[0]);

/* ---------------------------------------------------------
   Internal control structure
   --------------------------------------------------------- */
typedef struct {
    double  wave_timer; /* 距離下一波刷新的剩餘秒數 */
    int     wave_idx;   /* 保留：腳本波次 (未用)    */
} SpawnCtrl;

static SpawnCtrl _ctrl; /* 單例 */

/* ---------------------------------------------------------
   Helper: 建立單一怪物
   --------------------------------------------------------- */
static Elements *create_monster(MonsterType type, float x, float y)
{
    
    switch (type) {
        case MON_TUNGTUNGTUNG:  return New_tungtungtung(tungtungtung_L);
        case MON_TRIPPI_TROPPI: return New_trippi_troppi(trippi_troppi_L);
        case MON_BALL:          return New_Ball(Ball_L);
        case MON_SUSU:          return New_susu(Susu_L);
        case MON_CAPUCCINO:     return New_capuccino(capuccino_L);
        case MON_BANANINI:      return New_bananini(bananini_L);
        case MON_PATAPIM:       return New_patapim(patapim_L) ;
        case MON_TRALALA:       return New_tralala(tralala_L);
        case MON_CROCODILO:     return New_crocodilo(crocodilo_L);
        case MON_BIGTUNG:       return New_bigtung(bigtung_L);
        default:                return NULL;
    }
}

/* ---------------------------------------------------------
   Helper: 統計場上存活的主要怪物 (tung & trippi)
   --------------------------------------------------------- */
static int count_alive_monsters(Scene *scene)
{
    int cnt = 0;
    ElementVec all = _Get_all_elements(scene);
    for (int i = 0; i < all.len; ++i) {
        switch (all.arr[i]->label) {
            case tungtungtung_L:
            case trippi_troppi_L:
            case capuccino_L:
            case bananini_L:
            case patapim_L:
            case tralala_L:
            case crocodilo_L:
            case bigtung_L:
                ++cnt;
                break;
            default:
                break;
        }
    }
    return cnt;
}

/* ---------------------------------------------------------
   Public API: 外部呼叫
   --------------------------------------------------------- */
void MF_Spawn(Scene *scene, MonsterType type, float x, float y)
{
    Elements *m = create_monster(type, x, y);
    if (m) _Register_elements(scene, m);
}

void MF_Update(Scene *scene, double dt)
{
    const float INTERVAL = 2.0f;          /* 清場後 n 秒才刷下一波 */

    int alive = count_alive_monsters(scene);

    if (alive == 0) {                     /* 場上已經沒有 tung / trippi */
        _ctrl.wave_timer -= dt;

        if (_ctrl.wave_timer <= 0.0f) {
            /* 1) 還有腳本波可以播？ */
            if (_ctrl.wave_idx < g_wave_cnt) {
                const Wave *wave = &g_waves[_ctrl.wave_idx];

                /* 2) 依腳本生怪 */
                for (int i = 0; i < wave->entry_num; ++i) {
                    const SpawnEntry *e = &wave->entries[i];
                    for (int n = 0; n < e->count; ++n)
                        MF_Spawn(scene, e->type, 0, 0);   /* TODO: 替換隨機座標 */
                }

                ++_ctrl.wave_idx;        /* 3) 指向下一波 */
            } else {
                /* 播完最後一波 → 重新循環（也可改成停住） */
                _ctrl.wave_idx = 0;
            }

            _ctrl.wave_timer = INTERVAL; /* 重置倒數 */
        }
    } else {
        _ctrl.wave_timer = INTERVAL;     /* 場上還有怪，保持倒數凍結 */
    }
}


void MF_Reset(void)
{
    _ctrl.wave_timer = 0.0; /* 啟動時立即刷第一波 */
    _ctrl.wave_idx   = 0;
}

void MF_Destroy(void)
{
    /* 未配置資源，留空 */
}

bool MF_NextWaveIsBuffer(void)
{
    return (_ctrl.wave_idx < g_wave_cnt) ? g_waves[_ctrl.wave_idx].is_buffer : false;
}
void MF_SkipBufferWave(void)
{
    if (_ctrl.wave_idx < g_wave_cnt && g_waves[_ctrl.wave_idx].is_buffer)
        ++_ctrl.wave_idx;           /* 跳過佔位波 */
}

/* 暴露場上活怪數量，供 LevelFSM 判斷 */
int MF_AliveMonsterCount(Scene *scene)
{
    return count_alive_monsters(scene); /* 原本的 static 函式 */
}