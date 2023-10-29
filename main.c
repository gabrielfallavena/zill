#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

#define WIDTH 1200
#define HEIGHT 800
#define FPS 60
#define MAX_MONSTROS 10
#define MAP_WIDTH (WIDTH/50)
#define MAP_HEIGHT (HEIGHT/50)
#define N_LEVELS 2 //PODE-SE ADICIONAR QUANTOS NIVEIS QUISER, DESDE QUE MUDE NA CONSTANTE N_LEVELS
#define MAXSCORES 5
#define MAX_INPUT_CHARS 10

//TIPOS DE VARIÁVIES CRIADOS AO LONGO O DESENVOLVIMENTO DO JOGO
typedef enum {MENU, GAMEPLAY, GAMEOVER, ENDGAME, NEXT_LEVEL, CLEAR_GAME, RECORDES, VICTORY} GameState;

typedef enum {TRUE, FALSE} IsAlive;

typedef enum {NORTE, SUL, LESTE, OESTE} Direcao;

//ESTRUTURA DO JOGO - CONTÉM  VARIÁVIES DE ESTADO, TEXTOS, FONTES, SONS E NOMES DE ARQUIVOS
typedef struct{
    int selectedOption = 0;
    GameState state = MENU;
    int cont_enemy = 0;
    int dead_enemies = 0;
    Rectangle input_name = { WIDTH / 2 - 150, HEIGHT / 2 - 25, 300, 50};
    char arq_recorde[20] = "ranking.bin";
    const char *options[3] = {
        "Novo Jogo",
        "Scoreboard",
        "Sair"
    };
    const char *strings[10] ={
        "GAME OVER!!!",
        "Voce perdeu uma vida! Pressione ENTER para jogar novamente",
        "Pressione ENTER para voltar ao menu",
        "Parabens!!! Prepare-se para a proxima fase!",
        "PARABENS!!!",
        "*SCOREBOARD*",
        "Digite seu nome:"
    };

    const char* fonts[1] = {
        "OLDENGL.TTF",
    };

    Sound menu_theme = LoadSound("./sounds/menu_theme.mp3");
    Sound gameplay_theme = LoadSound("./sounds/overworld_theme.mp3");
    Sound death_sound = LoadSound("./sounds/death_sound.mp3");
    Sound victory_theme = LoadSound("./sounds/victory_theme.mp3");
    Sound dano = LoadSound("./sounds/dano.mp3");

    Color bgColor = { 30, 30, 30, 255 };
    Color textColor = { 255, 255, 255, 255 };
    Color titleColor = { 255, 220, 0, 255 };
    Color highlightColor = { 205, 175, 0, 255 };
    Color bronze = {237, 50, 0 , 200};
}GAME;

//ESTRUTURA DO PLAYER - CONTÉM TODOS CAMPOS NECESSÁRIOS PARA TRABALHAR ESSA ENTIDADE
typedef struct{
    Rectangle rec;
    Rectangle spawn;
    int life;
    float score;
    Vector2 vel;
    Texture2D textures[4];
    Texture2D textura_atual;
    Direcao direcao = SUL;
    Sound attack;
    int level_atual = 1;
    char nome_player[45];
}PLAYER;

//ESTRUTURA DO MUNDO - CONTEM A MATRIZ DOS NIVEIS, AS TEXTURAS DO AMBIENTE E SETA AS PAREDES
typedef struct{
    char map[MAP_HEIGHT][MAP_WIDTH];
    Rectangle parede[MAP_HEIGHT][MAP_WIDTH];
    Texture2D textures[2] = {LoadTexture("./sprites/world_sprites/Ground.png"), LoadTexture("./sprites/world_sprites/Obstacle.png")};
}WORLD;

//ESTRUTURA DO INIMGO - CONTÉM TODOS CAMPOS NECESSÁRIOS PARA TRABALHAR ESSA ENTIDADE
typedef struct{
    Rectangle rec;
    Rectangle spawn;
    int scale;
    Vector2 vel;
    Texture2D textures[4];
    Texture2D textura_atual;
    IsAlive is_alive = TRUE;
    Direcao direcao = SUL;
}ENEMY;

//ESTRUTURA DA ESPADA - CONTÉM CAMPOS NECESSÁRIOS PARA O FUNCIONAMENTO DA ESPADA
typedef struct{
    Rectangle rec;
    Texture2D textures[4];
    Texture2D *textura_atual;
}SWORD;

//ESTRUTURA DA SCOREBOARD - CADA PLAYER NO TOP5 CONTÉM NOME E PONTUAÇÃO
typedef struct{
    char nome[45] = "\0";
    int pontos;
}SCOREBOARD;

//FAZ SETUP DO PLAYER, REINICIALIZANDO VARIAVEIS E SETANDO POSIÇÃO INICIAL DE ACORDO COM A POSIÇÃO INDICADA PELO ARQUIVO .TXT
void setup_player(PLAYER *player){
    char nome_end[40];
    player->rec = (Rectangle){
        x: 0,
        y: 0,
        width: 50,//player->textura_atual->width,
        height: 50//player->textura_atual->height,
    };
    player->vel = (Vector2)
    {
        x:  (float) 300 / FPS,
        y:  (float) 200 / FPS
    };
    for(int i = 0; i < 4; i++){
        sprintf(nome_end, "./sprites/player_sprites/%d.png", i);
        player->textures[i] = LoadTexture(nome_end);
    }
    player->textura_atual = player->textures[1];
    player->life = 3;
    player->score = 0;
    player->level_atual = 1;
    for(int i = 0; i < strlen(player->nome_player); i++){
        player->nome_player[i] = ' ';
    }
}

//LÓGICA DE MOVIMENTAÇÃO DO PLAYER, VERIFICANDO COLISÃO COM PAREDE E COM INIMIGOS(MUDA O ESTADO DO GAME.STATE E PERDE VIDA EM COLISÃO COM INIMIGO)
void move_player(PLAYER *player, Rectangle parede[MAP_HEIGHT][MAP_WIDTH], ENEMY *enemy, GAME *game){
    if(IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)){
        player->rec.y -= player->vel.y;
        player->textura_atual = player->textures[0];
        player->direcao = NORTE;

        for(int i = 0; i < MAP_HEIGHT; i++){
            for(int j = 0; j < MAP_WIDTH; j++){
                while(CheckCollisionRecs(player->rec, parede[i][j])){
                    player->rec.y++;
                    player->rec.y = floor(player->rec.y);
                }
            }
        }
    }
    if(IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)){
        player->rec.y += player->vel.y;
        player->textura_atual = player->textures[1];
        player->direcao = SUL;
        for(int i = 0; i < MAP_HEIGHT; i++){
            for(int j = 0; j < MAP_WIDTH; j++){
                while(CheckCollisionRecs(player->rec, parede[i][j])){
                    player->rec.y--;
                    player->rec.y = ceil(player->rec.y);
                }
            }
        }
    }
    if(IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)){
        player->rec.x -= player->vel.x;
        player->textura_atual = player->textures[2];
        player->direcao = OESTE;
        for(int i = 0; i < MAP_HEIGHT; i++){
            for(int j = 0; j < MAP_WIDTH; j++){
                while(CheckCollisionRecs(player->rec, parede[i][j])){
                    player->rec.x++;
                    player->rec.x = floor(player->rec.x);
                }
            }
        }
    }
    if(IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)){
        player->rec.x += player->vel.x;
        player->textura_atual = player->textures[3];
        player->direcao = LESTE;
        for(int i = 0; i < MAP_HEIGHT; i++){
            for(int j = 0; j < MAP_WIDTH; j++){
                while(CheckCollisionRecs(player->rec, parede[i][j])){
                    player->rec.x--;
                    player->rec.x = ceil(player->rec.x);
                }
            }
        }
    }
    for(int i = 0; i < game->cont_enemy; i++){
        if(CheckCollisionRecs(player->rec, enemy[i].rec)){
            player->life -= 1;
            if(player->life == 0)
                game->state = ENDGAME;
            else
                game->state = GAMEOVER;
            break;
        }
    }
}

//FUNÇÃO QUE RETORNA A POSIÇÃO XY DO RETANGULO DO JOGADOR
Vector2 player_pos(PLAYER player){
    return (Vector2){
        player.rec.x, player.rec.y
    };
}

//SETUP DO INIMIGO, REINICILIZANDO VARIÁVIES DE ESTADO E SETANDO POSIÇÃO INICIAL E DE SPAWN
void setup_enemy(ENEMY *enemy){

    char nome_end[40];

    enemy->rec = (Rectangle){
        x: 100,
        y: 100,
        width: 50,
        height: 50
    };
    enemy->vel = (Vector2){
        x:  (float) 100 / FPS,
        y:  (float) 50 / FPS
    };

    for(int i = 0; i < 4; i++){
        sprintf(nome_end, "./sprites/enemy_sprites/%d.png", i);
        enemy->textures[i] = LoadTexture(nome_end);
    }

    enemy->textura_atual = enemy->textures[1];
    enemy->is_alive = TRUE;
}

//LÓGICA DE MOVIMENTAÇÃO DO INIMIGO(PERSEGUEM O PLAYER) E VERIFICAÇÃO DE COLISÃO COM PLAYER E PAREDES
void move_enemy(ENEMY *enemy, PLAYER *player, Rectangle parede[MAP_HEIGHT][MAP_WIDTH]){
    if(enemy->rec.y >= player->rec.y){
        enemy->rec.y -= enemy->vel.y;
        enemy->textura_atual = enemy->textures[0];
        for(int i = 0; i < MAP_HEIGHT; i++){
            for(int j = 0; j < MAP_WIDTH; j++){
                while(CheckCollisionRecs(enemy->rec, parede[i][j])){
                    enemy->rec.y++;
                    enemy->rec.y = floor(enemy->rec.y);
                }
            }
        }
    }
    if(enemy->rec.y <= player->rec.y){
        enemy->rec.y += enemy->vel.y;
        enemy->textura_atual = enemy->textures[1];
        for(int i = 0; i < MAP_HEIGHT; i++){
            for(int j = 0; j < MAP_WIDTH; j++){
                while(CheckCollisionRecs(enemy->rec, parede[i][j])){
                    enemy->rec.y--;
                    enemy->rec.y = ceil(enemy->rec.y);
                }
            }
        }
    }
    if(enemy->rec.x >= player->rec.x){
        enemy->rec.x -= enemy->vel.x;
        enemy->textura_atual = enemy->textures[2];
        for(int i = 0; i < MAP_HEIGHT; i++){
            for(int j = 0; j < MAP_WIDTH; j++){
                while(CheckCollisionRecs(enemy->rec, parede[i][j])){
                    enemy->rec.x++;
                    enemy->rec.x = floor(enemy->rec.x);
                }
            }
        }
    }
    if(enemy->rec.x <= player->rec.x){
        enemy->rec.x += enemy->vel.x;
        enemy->textura_atual = enemy->textures[3];
        for(int i = 0; i < MAP_HEIGHT; i++){
            for(int j = 0; j < MAP_WIDTH; j++){
                while(CheckCollisionRecs(enemy->rec, parede[i][j])){
                    enemy->rec.x--;
                    enemy->rec.x = ceil(enemy->rec.x);
                }
            }
        }
    }

    if(floor(enemy->rec.x) == floor(player->rec.x) && enemy->rec.y < player->rec.y)
        enemy->textura_atual = enemy->textures[1];
    if(floor(enemy->rec.x) == floor(player->rec.x) && enemy->rec.y > player->rec.y)
        enemy->textura_atual = enemy->textures[0];
}

//RETORNA XY DO RETANGULO DO INIMIGO
Vector2 enemy_pos(ENEMY enemy){
    return (Vector2){
        enemy.rec.x, enemy.rec.y
    };
}

//SETUP DA ESPADA
void setup_sword(SWORD *sword){

     char nome_end[40];

    sword->rec = (Rectangle){
        x: 100,
        y: 100,
        width: 50,
        height: 50
    };
    for(int i = 0; i < 4; i++){
        sprintf(nome_end, "./sprites/sword_sprites/%d.png", i);
        sword->textures[i] = LoadTexture(nome_end);
    }
    sword->textura_atual = &sword->textures[1];
}

//DESENHA A ESPADA NO BLOCO A FRENTE DO JOGADOR AO APERTAR A TECLA "J".
void draw_sword(SWORD *sword, PLAYER *player){

    Vector2 sword_pos ={0,0};

    if(IsKeyReleased(KEY_J)){
        switch(player->direcao){
            case NORTE:
                sword->rec.x = player->rec.x;
                sword->rec.y = player->rec.y-50;
                sword_pos = {sword->rec.x, sword->rec.y};
                DrawTextureV(sword->textures[3], sword_pos, WHITE);
                break;
            case SUL:
                sword->rec.x = player->rec.x;
                sword->rec.y = player->rec.y+50;
                sword_pos = {sword->rec.x, sword->rec.y};
                DrawTextureV(sword->textures[0], sword_pos, WHITE);
                break;
            case LESTE:
                sword->rec.x = player->rec.x+50;
                sword->rec.y = player->rec.y;
                sword_pos = {sword->rec.x, sword->rec.y};
                DrawTextureV(sword->textures[2], sword_pos, WHITE);
                break;
            case OESTE:
                sword->rec.x = player->rec.x-50;
                sword->rec.y = player->rec.y;
                sword_pos = {sword->rec.x, sword->rec.y};
                DrawTextureV(sword->textures[1], sword_pos, WHITE);
                break;
        }
    }else{
        sword->rec.x = 0;
        sword->rec.y = 0;
    }
}

//FUNÇÃO DE ATAQUE DA ESPADA. DEVE-SE SOLTAR A TECLA "J" PARA QUE SEJA FEITO O ATAQUE.
//EM COLISÃO COM INIMGO, ELIMINA ESSE INIMIGO DA GAMEPLAY
void attack_sword(PLAYER *player, SWORD *sword, ENEMY enemy[],GAME *game){
    for(int i = 0; i < game->cont_enemy; i++){
        if(CheckCollisionRecs(sword->rec, enemy[i].rec)){
            PlaySound(game->dano);
            player->score+=50;
            enemy[i].is_alive = FALSE;
            game->dead_enemies++;
        }
    }
}

//DESENHA TEXTOS E TEXTURAS DO MENU
void draw_menu(GAME *game, PLAYER *player, ENEMY *enemy, SWORD *sword){
    BeginDrawing();
    ClearBackground(game->bgColor);
    Font fonte_titulo = LoadFont("./fonts/ALGER.TTF");
    Vector2 vector_titulo = {500, 100};
    Vector2 vector_link = {800,400};
    Vector2 vector_sword = {500,140};

    DrawTextureEx(player->textures[1], vector_link, 0, 5, WHITE);
    DrawTextureEx(sword->textures[1], vector_sword, 0, 5, WHITE);
    DrawTextEx(fonte_titulo, "Zill", vector_titulo, 150, 1, game->titleColor);

    for (int i = 0; i < 3; i++) {
        if (i == game->selectedOption)
            DrawText(game->options[i], 100,400+100 * i, 40, game->highlightColor);
        else
            DrawText(game->options[i], 100,400+100 * i, 40, game->textColor);
    }
}

//UPDATE DO MENU, COM OPÇÕES DE INICIAR JOGO, VER SCOREBOARD E SAIR
void logica_menu(GAME *game){

    if ((IsKeyPressed(KEY_DOWN)||IsKeyPressed(KEY_S)) && game->selectedOption < 2)
        game->selectedOption++;
    else if ((IsKeyPressed(KEY_UP)||IsKeyPressed(KEY_W)) && game->selectedOption > 0)
        game->selectedOption--;

    if (IsKeyPressed(KEY_ENTER)) {
        switch (game->selectedOption) {
            case 0:
                game->state = GAMEPLAY;
                break;
            case 1:
                game->state = RECORDES;
                break;
            case 2:
                CloseWindow();
                break;
            default:
                break;
        }
    }
}

//LÊ ARQUIVO TEXTO E PASSA O MAPA PARA UMA MATRIZ, ALEM DE CRIAR UM ARRAY DE RETANGULOS(PARADES). SETA AS POSIÇÕES DE
//SPAWN DO PLAYER E DOS INIMGOS ALÉM DE GERENCIAR CONTAGEM DE INIMIGOS INICIAIS DA FASE(ÚTIL PARA DETERMINAR O NEXTLEVEL)
void setup_world(WORLD *world, PLAYER *player, ENEMY enemy[], GAME *game){

    FILE *arquivo;
    char caractere;
    char nome_arquivo[40];

    game->cont_enemy = 0;
    game->dead_enemies = 0;

    sprintf(nome_arquivo, "./levels/nivel%d.txt", player->level_atual);

    arquivo = fopen(nome_arquivo, "r");
    if (arquivo == NULL){
        printf("Erro ao abrir o arquivo.\n");
    }

    for (int i = 0; i < MAP_HEIGHT; i++) {
        for (int j = 0; j < MAP_WIDTH; j++) {
            if(fscanf(arquivo, " %c", &caractere) != 1) {
                printf("Erro ao ler o caractere.\n");
                fclose(arquivo);
            }
            world->map[i][j] = caractere;
            if(world->map[i][j] == 'O')
                world->parede[i][j] = {j*50, i*50, 50, 50};
            else if(world->map[i][j] == 'B')
                world->parede[i][j] = {0, 0, 0, 0};
            else if(world->map[i][j] == 'P'){
                player->rec.x = (j*50);
                player->spawn.x = (j*50);
                player->rec.y = (i*50);
                player->spawn.y = (i*50);
            }else if(world->map[i][j] == 'M'){
                setup_enemy(&enemy[game->cont_enemy]);
                enemy[game->cont_enemy].rec.x = (j*50);
                enemy[game->cont_enemy].spawn.x = (j*50);
                enemy[game->cont_enemy].rec.y = (i*50);
                enemy[game->cont_enemy].spawn.y = (i*50);
                game->cont_enemy++;
            }
        }
    }
    fclose(arquivo);
}

//DESENHA A TEXTURA DE ACORDO COM O CARACTERE DEFINIDO NO TXT DO MAPA
void draw_world(char matriz[MAP_HEIGHT][MAP_WIDTH], Texture2D map_texture[]){

    char symbol;

    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++){
            symbol = matriz[y][x];
            if(symbol == 'P')
                DrawTexture(map_texture[0], x * 50, y * 50, WHITE);
            else if(symbol == 'B')
                DrawTexture(map_texture[0], x * 50, y * 50, WHITE);
                //DrawRectangle(x * 50, y * 50, 50, 50, YELLOW);
            else if(symbol == 'O')
                DrawTexture(map_texture[1], x * 50, y * 50, WHITE);
                //DrawRectangle(x * 50, y * 50, 50, 50, BLACK);
            else if(symbol == 'M')
                DrawTexture(map_texture[0], x * 50, y * 50, WHITE);
        }
    }
}

//DESENHA INTERFACE DE USUÁRIO NA GAMEPLAY, COM VIDAS RESTANTES E PONTUAÇÃO TOTAL ATUAL
void draw_ui(PLAYER *player){
    char texto[10];
    sprintf(texto, "Vidas: %d  Score: %2.2f", player->life, player->score);
    DrawRectangle(0, 0, 1200, 60, BLACK);
    DrawText(texto, 15, 15, 30, WHITE);

}

//FUNÇÃO USADA PARA REINICIAR A FASE, VOLTANDO PLAYER E INIMIGOS
//PARA POSIÇÕES DE SPAWN. MANTEM-SE OS INIMIGOS MORTOS E A PONTUAÇÃO DO JOGADOR
void restart_game(PLAYER *player, ENEMY enemy[], GAME *game){
    player->rec.x = player->spawn.x;
    player->rec.y = player->spawn.y;
    player->textura_atual = player->textures[1];
    for(int i = 0; i < game->cont_enemy; i++){
        enemy[i].rec.x = enemy[i].spawn.x;
        enemy[i].rec.y = enemy[i].spawn.y;
    }
    game->state = GAMEPLAY;
}

//FUNÇÃO DE AUDIO QUE MANTÉM MUSICAS TEMAS TOCANDO
void AmbientSound(Sound Ambiente){
    if(!IsSoundPlaying(Ambiente))
    {
        PlaySound(Ambiente);
    }
}

//UPDATE DA TELA DE OBTENÇÃO DO NOME DO PLAYER.
//ALTERA O SCOREBOARD CASO A PONTUAÇÃO SEJA TOP5, E REORDENA O TOP5
void logica_name(PLAYER *player, int *letterCount, bool *editing, SCOREBOARD top5[]){
    if (*editing){
        int key = GetKeyPressed();
        if (key == KEY_BACKSPACE && *letterCount > 0){
                player->nome_player[*letterCount - 1] = '\0';
                (*letterCount)--;
        }

        if (key != NULL && *letterCount < MAX_INPUT_CHARS){
            if (((key >= 32) && (key <= 125))){
                player->nome_player[*letterCount] = (char)key;
                (*letterCount)++;
                printf("\n%s\n", player->nome_player);
            }
        }
        if (IsKeyPressed(KEY_ENTER)){
            *editing = true;
            player->nome_player[*letterCount+1] = '\0';
            *letterCount = 0;

            int insertIndex = -1;
            for (int i = 0; i < MAXSCORES; i++) {
                if(player->score > top5[i].pontos) {
                    insertIndex = i;
                    break;
                }
            }
            if (insertIndex >= 0) {
                for (int i = MAXSCORES - 1; i > insertIndex; i--) {
                    top5[i] = top5[i - 1];
                }
                top5[insertIndex].pontos = player->score;
                strcpy(top5[insertIndex].nome, player->nome_player);
            }
        }
    }
}

//DESENHA A TELA DE OBTENÇÃO DO NOME, MOSTRANDO TAMBÉM A PONTUAÇÃO FEITA PELO JOGADOR
void draw_name(PLAYER *player, GAME *game, bool editing){

    ClearBackground(game->bgColor);
    DrawText(game->strings[6], WIDTH/2- MeasureText(game->strings[6], 30)/2, game->input_name.y - 40, 30, game->highlightColor);
    DrawRectangleLinesEx(game->input_name, 2, WHITE);
    DrawText(player->nome_player, game->input_name.x + 5, game->input_name.y + 8, 40, game->highlightColor);

    char texto[20];
    sprintf(texto, "Sua pontuacao: %2.2f", player->score);
    DrawText(texto, WIDTH/2 - MeasureText(texto, 30)/2, 500, 30, WHITE);
}

//OBTÉM A PONTUAÇÃO PREVIAMENTE SALVA DO RANKING.BIN, E PASSA PARA O SCOREBOARD TOP5
void le_arquivo(char nome_arq[], SCOREBOARD top5[MAXSCORES]){

    FILE *fp;
    int resultado = 0;

    if(!(fp = fopen(nome_arq, "rb"))){
        printf("Erro na abertura\n");
    }
    else{
        if(fread(top5, sizeof(SCOREBOARD), MAXSCORES ,fp)== MAXSCORES){
            resultado = 1;
        }
    }
    fclose(fp);

}

//SALVA O SCOREBOARD NO ARQUIVO RANKING.BIN
int salva_arquivo(char nome_arq[], SCOREBOARD top5[MAXSCORES]){
    FILE *fp;
    int resultado = FALSE;


    if(!(fp = fopen(nome_arq,"wb"))){
        printf("Erro na abertura\n");
    }
    else{
        if(fwrite(top5, sizeof(SCOREBOARD), MAXSCORES ,fp) != 0){
            resultado = TRUE;
        }
    }
    fclose(fp);

    return resultado;
}

//DESENHA O NOME E A PONTUAÇÃO DO TOP5
void draw_scoreboard(GAME *game, SCOREBOARD top5[]){
    char j[45];
    ClearBackground(game->bgColor);
    DrawText(game->strings[5], (WIDTH/2)-(MeasureText(game->strings[5], 70)/2), 100, 70, YELLOW);
    for(int k = 0; k < MAXSCORES; k++){
        sprintf(j, "%s --------------> %d", top5[k].nome, top5[k].pontos);
        switch(k){
            case 0: DrawText(j, 225, 250+(k*70), 50, GOLD);
                break;
            case 1: DrawText(j, 225, 250+(k*70), 50, GRAY);
                break;
            case 2: DrawText(j, 225, 250+(k*70), 50, game->bronze);
                break;
            default: DrawText(j, 225, 250+(k*70), 50, WHITE);
                break;
        }
    }
    DrawText(game->strings[2], (WIDTH/2)-(MeasureText(game->strings[2], 30)/2), 700, 30, YELLOW);
}

//UPDATE DO GAME - GIRA EM TORNO DO ESTADO DO GAME.STATE
void update_game(GAME *game, PLAYER *player, ENEMY enemy[], WORLD *world, SCOREBOARD top5[], SWORD *sword, int *flag, int *letterCount, bool *editing){
    switch (game->state){
            case MENU: //CIRCULANDO PELO MENU
                if(IsSoundPlaying(game->gameplay_theme))
                    StopSound(game->gameplay_theme);
                logica_menu(game);
                AmbientSound(game->menu_theme);
                break;
            case GAMEPLAY: //PLAYER JOGANDO
                if(*flag == 0 && player->level_atual <= N_LEVELS){
                    setup_world(world, player, enemy, game);
                    *flag = 1;
                }else if(player->level_atual > N_LEVELS){
                    game->state = CLEAR_GAME;
                    break;
                }
                StopSound(game->menu_theme);
                move_player(player, world->parede, enemy, game);
                for(int x = 0; x < game->cont_enemy; x++){
                    move_enemy(&enemy[x], player, world->parede);
                    if(enemy[x].is_alive == FALSE)
                        enemy[x].rec = {0,0,0,0};
                }
                if(game->dead_enemies == game->cont_enemy){
                    player->level_atual+=1;
                    *flag = 0;
                    game->dead_enemies = 0;
                    if(player->level_atual > N_LEVELS){
                        game->state = VICTORY;
                        break;
                    }
                    game->state = NEXT_LEVEL;
                }
                attack_sword(player, sword, enemy, game);
                AmbientSound(game->gameplay_theme);
                break;
            case NEXT_LEVEL: //PLAYER PASSOU DE FASE
                if(IsKeyPressed(KEY_ENTER))
                    game->state = GAMEPLAY;
                break;
            case CLEAR_GAME: //APÓS A PARTIDA, OBTÉM O NOME DO PLAYER
                if(IsSoundPlaying(game->gameplay_theme))
                    StopSound(game->gameplay_theme);
                logica_name(player, letterCount, editing, top5);
                if(IsKeyPressed(KEY_ENTER)){
                    salva_arquivo(game->arq_recorde, top5);
                    setup_player(player);
                    setup_sword(sword);
                    *flag = 0;
                    game->state = MENU;
                }
                break;
            case GAMEOVER: //PLAYER PERDEU UMA VIDA
                if(IsSoundPlaying(game->gameplay_theme))
                    StopSound(game->gameplay_theme);
                AmbientSound(game->death_sound);
                if(IsKeyPressed(KEY_ENTER))
                    restart_game(player, enemy, game);
                break;
            case ENDGAME: //PLAYER PERDEU TODAS VIDAS
                if(IsSoundPlaying(game->gameplay_theme))
                    StopSound(game->gameplay_theme);
                AmbientSound(game->death_sound);
                if(IsKeyPressed(KEY_ENTER))
                    game->state = CLEAR_GAME;
                break;
            case VICTORY: //PLAYER VENCEU TODAS AS FASES
                if(IsSoundPlaying(game->gameplay_theme))
                    StopSound(game->gameplay_theme);
                AmbientSound(game->victory_theme);
                if(IsKeyPressed(KEY_ENTER))
                    game->state = CLEAR_GAME;
                break;
            case RECORDES: //MOSTRA O SCOREBOARD
                if(IsKeyPressed(KEY_ENTER))
                    game->state = MENU;
                break;
            default:
                break;
    }
}

//DESENHA O GAME - GIRA EM TORNO DO ESTADO DO GAME.STATE
void draw_game(GAME *game, PLAYER *player, ENEMY enemy[], WORLD *world, SCOREBOARD top5[], SWORD *sword, int *flag, int *letterCount, bool *editing){
    switch (game->state){
            case MENU: //CIRCULANDO PELO MENU
                draw_menu(game, player, enemy, sword);
                break;
            case GAMEPLAY: //PLAYER JOGANDO
                draw_world(world->map, world->textures);
                DrawTextureV(player->textura_atual, player_pos(*player), WHITE);
                for(int k = 0; k < game->cont_enemy; k++)
                    DrawTextureV(enemy[k].textura_atual, enemy_pos(enemy[k]), WHITE);
                draw_sword(sword, player);
                draw_ui(player);
                break;
            case NEXT_LEVEL: //PLAYER PASSOU PARA PRÓXIMA FASE
                ClearBackground(game->bgColor);
                DrawText(game->strings[3], (WIDTH/2)-(MeasureText(game->strings[3], 30)/2), 420, 30, game->highlightColor);
                break;
            case CLEAR_GAME: //APÓS A PARTIDA, OBTÉM O NOME DO PLAYER
                draw_name(player, game, editing);
                break;
            case GAMEOVER: //PLAYER PERDEU UMA VIDA
                ClearBackground(game->bgColor);
                DrawText(game->strings[1], (WIDTH/2)-(MeasureText(game->strings[1], 30)/2), 420, 30, game->highlightColor);
                break;
            case ENDGAME: //PLAYER PERDEU TODAS VIDAS
                ClearBackground(game->bgColor);
                DrawText(game->strings[0], (WIDTH/2)-(MeasureText(game->strings[0], 80)/2), (HEIGHT/2)-40, 80, WHITE);
                DrawText(game->strings[2], (WIDTH/2)-(MeasureText(game->strings[2], 20)/2), (HEIGHT/2) +45, 20, WHITE);
                break;
            case VICTORY: //PLAYER VENCEU TODAS AS FASES
                ClearBackground(game->bgColor);
                DrawText(game->strings[4], (WIDTH/2)-(MeasureText(game->strings[4], 80)/2), (HEIGHT/2)-40, 80, GREEN);
                break;
            case RECORDES: //MOSTRA O SCOREBOARD
                draw_scoreboard(game, top5);
                break;
            default:
                break;
    }
}

//LÓGICA CENTRAL DO JOGO
int main() {
    InitWindow(WIDTH, HEIGHT, "Game");
    SetTargetFPS(FPS);
    InitAudioDevice();

    GAME game;
    WORLD world;
    PLAYER player;
    ENEMY enemy[MAX_MONSTROS];
    SWORD sword;
    SCOREBOARD top5[MAXSCORES];

    le_arquivo(game.arq_recorde, top5);
    setup_player(&player);
    setup_sword(&sword);

    int flag = 0;
    int letterCount = 0;
    bool editing = true;

    while (!WindowShouldClose()) {
        //ATUALIZA LÓGICA DO JOGO
        update_game(&game, &player, enemy, &world, top5, &sword, &flag, &letterCount, &editing);
        //ATUALIZA PARTE GRÁFICA DO JOGO
        draw_game(&game, &player, enemy, &world, top5, &sword, &flag, &letterCount, &editing);

        EndDrawing();
    }
    CloseWindow();
    return 0;
}
