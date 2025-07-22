#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define LARGURA 1200 //Largura da tela
#define ALTURA 800 //Altura da tela, na partida são 200 de HUD e 600 do jogo no meio
#define NUMERO_SAVES 3 //Numero de saves do jogo
#define DIMENSOES_SAVE 300 //Dimensões padrões da textura dos saves
#define HUD 100 //HUD dentro da partida em si, 100 na parte superior e 100 na inferior, eqHUDvale a ALTURA / 8
#define LINHAS 30 //Linhas dos mapas
#define COLUNAS 60 //Colunas dos mapas
#define QUADRADO 20 //EqHUDvale a LARGURA / (COLUNAS -1) e a ALTURA - (2 * HUD) / LINHAS
#define GRAVIDADE 5 //Gravidade padrão
#define VELOCIDADE_PULO 5 //Velocidade do pulo, mostra quanto o personagem vai subir em um pulo por frame
#define MAX_PULO 140 //Altura máxima total de um pulo, pode ser ultrapassado com certos valores de pulo mas não por mHUDto. MAX_PULO / 20 = quantos blocos de altura valem um pulo maximo
#define VELOCIDADE_MOVIMENTO 4 //Velocidade do movimento horizontal
#define TEMPO_AVISO 600 //Tempo que representa os segundos (divido por 60) entre cada aviso na tela de Controles
#define VIDA_TOTAL_INICIAL 3 //Total de vidas que determina quantas vezes o jogador pode morrer antes de um game over
#define MAX_MONSTROS 15 //Numero maximo de monstros por mapa
#define VELOCIDADE_MONSTROS_PADRAO 1 //Velocidade padrão que os monstros se movimentam
#define MONSTROS_NOVO_SENTIDO 300 //Tempo em frames (60 fps) para uma nova direção do monstro
#define MAX_OBJETOS 3 //Numero maximo de objetos que podem ser arremessados por vez
#define VELOCIDADE_OBJETO 4 //Velocidade do objeto arremessado
#define COLETAVEIS_PARA_OBJETO 10 //Numero de coletaveis necessários para um objeto
#define PONTOS_MORTE -100 //Pontuação ao morrer ou reiniciar
#define PONTOS_MONSTRO 50 //Pontuação ao matar monstros

typedef struct Saves //Caracteristicas relativas aos saves
{
    int contador; //Contador de cada frame da animação
    int lido; //Flag que diz se o save já foi lido ou não
    int opcao;
    char atual[10]; //Nome do arquivo do save selecionado
    Rectangle textura[NUMERO_SAVES]; //Posição e tamanho das imagens dos saves
    Sound selec1; //Audio ao abrir save1
    Sound selec2; //Audio ao abrir save1
    Sound selec3; //Audio ao abrir save3
} SAVES;

typedef struct Monstro //Caracteristicas relativas aos monstros
{
    int sentido; //Sentido para qual o Monstro se movimenta, pode ser 1 ou -1
    int contador; //Contagem para que o sentido do monstro muda
    int velocidade; //Distância que o monstro se move a cada quadro
	Rectangle corpo; //Posição do Monstro na tela e seu tamanho
} MONSTRO;

typedef struct Jogador //Caracteristicas relativas ao jogador
{
    Rectangle corpo; //Posição de Jogador na tela e seu tamanho
    Rectangle objetos[MAX_OBJETOS]; //Contém a posição e tamanho de cada objeto do jogador
    int objetosArremessados; //Numero de objetos na tela
    int	vida, vidaTotal; //Determina se houve colisão com espinhos ou monstros, e quantas vezes o jogador pode colidir com os mesmos antes do "game over"
	int velocidadePulo; //Valor que o personagem vai subir por frame
	int velocidade; //Distância que o Jogador se move a cada quadro
	int pulo, puloMax, puloAux; //Determinam se o pulo pode ser executado, a altura do pulo em relação ao máximo e se a gravidade pode voltar a ser aplicada
	int coletaveis; //Contagem de coletáveis coletados na fase atual
	int pontuacao, pontuacaoFaseAtual; //Número de pontos conqHUDstados dentre todas fases e na fase atual
	int faseAtual; //Fase em que o jogador está
	int reliquias, reliquiaFaseAtual; //Contagem de reliquias coletadas no save e na fase atual
	int contador; //Contagem para animações
	int direcao; //Pode ser -1 ou 1, serve para as animações
	int modoDeus; //Vidas infinitas, projeteis infinitos
	char sentido; //Sentido que o Jogador está virado e dará o soco
	char sentidoObjetos[MAX_OBJETOS];
    Texture skin; //Textura de Jogador
    Texture soco; //Textura do soco do Jogador
    Texture objeto; //Textura do objeto arremessavel
    Sound morte; //Efeito sonoro da morte do jogador
    Sound falhaObjeto; //Efeito sonoro de clicar X sem coletáveis o suficiente
    Sound arramessarObjeto; //Efeito sonoro ao arremessar objeto
    Sound socar; //Efeito sonoro de apertar Z
} JOGADOR;

typedef struct Portal //Caracteristicas relativas ao portal
{
    Texture portal; //Textura do Portal
    Vector2 pos; //Posição do Portal
    int contador; //Contador para a animação do Portal
} PORTAL;

typedef struct Reliquia //Caracteristicas relativas à Reliquia
{
    Texture reliquia; //Textura da Reliquia
    Rectangle pos; //Posições e tamanho da Reliquia
} RELIQUIA;

//Função MoveMonstros
//Recebe um Monstro, o array contendo as posições na tela e os tamanhos de um quadrado e do HUD e move o monstro para o seu sentido. A cada tempoNovoSentido, gera um novo sentido (que pode ser 1 ou -1 ou em caso de colisão com paredes ou em risco de cair no ar, inverte o sentido do movimento.
void MoveMonstros(MONSTRO *monstro, char posicoesDaTela[][COLUNAS], int tempoNovoSentido)
{
    if(monstro->contador >= tempoNovoSentido) //A cada quantidade de tempo gera um novo sentido
    {
        do
        {
            monstro->sentido = GetRandomValue(-1, 1);//Gera -1, 0 ou 1, sendo aceitos apenas 1 e -1
        } while(monstro->sentido == 0); //O valor precisa ser necessariamente -1 ou 1, pois 0 o deixaria imóvel
        monstro->contador = 0; //Reincia o contador
    }
    if(posicoesDaTela[((int)monstro->corpo.y - HUD + QUADRADO) / QUADRADO][((int)monstro->corpo.x) / QUADRADO] == ' ' || posicoesDaTela[((int)monstro->corpo.y  - HUD + QUADRADO) / QUADRADO][((int)monstro->corpo.x + QUADRADO + monstro->velocidade) / QUADRADO] == ' ' || posicoesDaTela[((int)monstro->corpo.y - HUD) / QUADRADO][((int)monstro->corpo.x + QUADRADO + monstro->velocidade) / QUADRADO] == '#' || posicoesDaTela[((int)monstro->corpo.y - HUD) / QUADRADO][((int)monstro->corpo.x - monstro->velocidade) / QUADRADO] == '#' || posicoesDaTela[((int)monstro->corpo.y - HUD) / QUADRADO][((int)monstro->corpo.x + QUADRADO + monstro->velocidade) / QUADRADO] == 'B' || posicoesDaTela[((int)monstro->corpo.y - HUD) / QUADRADO][((int)monstro->corpo.x - monstro->velocidade) / QUADRADO] == 'B' || posicoesDaTela[((int)monstro->corpo.y - HUD) / QUADRADO][((int)monstro->corpo.x + QUADRADO + monstro->velocidade) / QUADRADO] == 'X' || posicoesDaTela[((int)monstro->corpo.y - HUD) / QUADRADO][((int)monstro->corpo.x - monstro->velocidade) / QUADRADO] == 'X')
    {//Verifica se  as posições do solo a frente e abaixo dos pés de monstro são ar ou se as posições diretamente a frente são paredes ou blocos destrutiveís
        monstro->sentido *= -1; //Caso sejam, inverte o sentido, evitando a colisão
        monstro->corpo.x += (monstro->velocidade * monstro->sentido); //Move o monstro no sentido atual
    }
    else //Caso não haja colisão, continua movendo o monstro
    {
        monstro->corpo.x += (monstro->velocidade * monstro->sentido); //Move o monstro no sentido atual
    }
    monstro->contador++; //A cada execução o contador vai aumentando para determinar quando 5 segundos se passarem
}

//Função Musica:
//Dados uma musica e uma variavel que representa se o jogo está mutado, toca música caso o jogo não esteja mutado
void Musica(Music som, int musica)
{
    if(musica == 1)//Caso a música não esteja mutada
    {
        PlayMusicStream(som);
        UpdateMusicStream(som);
    }
}

//Função Controles:
//Mostra ao jogador a tela dos controles, enquanto toca um aviso para que o jogo continue a cada 10 segundos. Caso uma tecla seja apertada, retorna 1, avisando que a função já foi exibida uma vez
int Controles(Music som, int musica, Texture textura, Sound introducao, Sound selecionar)
{
    int k = TEMPO_AVISO;//Começa em 600 para que  o audio seja tocado imeditamente
    while(!(IsKeyPressed(KEY_W) || IsKeyPressed(KEY_A) || IsKeyPressed(KEY_S) || IsKeyPressed(KEY_D) || IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_Z) || IsKeyPressed(KEY_R) || IsKeyPressed(KEY_X) || IsKeyPressed(KEY_ESCAPE)))
    {//Caso qualquer tecla dentre os controles seja pressionada sai do loop
        if(k == TEMPO_AVISO) //Constante que vale 600 e representa que se passaram 60 segundos
        {
            PlaySound(introducao);
            k = 0; //Reinicia o contador
        }
        k++; //Vai aumentando e contando o tempo da repetição
        BeginDrawing();
        Musica(som, musica);
        DrawTexture(textura, 0, 0, RAYWHITE); //Desenha a imagem com os controles
        EndDrawing();
    }
    PlaySound(selecionar);
    EndDrawing();
    return 1; //Retorna 1 para evitar que a tela de opções seja exibida mais de uma vez
}

//Função SelecionadorMenus
//Serve para navegar um menu, alterando o valor de uma variável de opção baseado na tecla clicada com um limite maximo e começando por 1
void SelecionadorMenus(int *opcao, int opcaoMax)
{
    if((IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) && *opcao > 1)
    {
        (*opcao)--;
    }
    else if((IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) && *opcao < opcaoMax)
    {
        (*opcao)++;
    }
}

//Função config
//Abre o menu de configurações, com as opções voltar, controles, dificuldade, mutar/desmutar, modo Deus e formatar
void Config(Music som, int *musica, Texture textura, Sound introducao, Sound selecionar, int *dificuldade, int *vida, int *modoDeus)
{
    int config = 1, opcaoConfig = 1, k;
    EndDrawing();
    while(config == 1)
    {
        BeginDrawing();
        Musica(som, *musica);
        ClearBackground(BLACK);
		//Desenho dos retângulos das opções
        DrawRectangle(LARGURA / 8, ALTURA * 1 / 10, LARGURA * 6 / 8, ALTURA / 12, LIME);
        DrawRectangle(LARGURA / 8 , ALTURA * 2.5 / 10, LARGURA * 6 / 8, ALTURA / 12, LIME);
        DrawRectangle(LARGURA / 8, ALTURA * 4 / 10, LARGURA * 6 / 8, ALTURA / 12, LIME);
        DrawRectangle(LARGURA / 8, ALTURA * 5.5 / 10, LARGURA * 6 / 8, ALTURA / 12, LIME);
        DrawRectangle(LARGURA / 8, ALTURA * 7 / 10, LARGURA * 6 / 8, ALTURA / 12, LIME);
        DrawRectangle(LARGURA / 8, ALTURA * 8.5 / 10, LARGURA * 6 / 8, ALTURA / 12, LIME);
        switch(opcaoConfig)
        {//Para cada opção selecionada, é desenhado um retângulo por cima do primeiro mostrando qual está selecionada
            case 1: //Opção voltar
                DrawRectangle(LARGURA / 8, ALTURA * 1 / 10, LARGURA * 6 / 8, ALTURA / 12, DARKGREEN);
                if(IsKeyPressed(KEY_ENTER))
                {
                    PlaySound(selecionar);
                    config = 0; //Quebra o laço e sai das configurações
                }
                break;
            case 2://Opção controles
                DrawRectangle(LARGURA / 8 , ALTURA * 2.5 / 10, LARGURA * 6 / 8, ALTURA / 12, DARKGREEN);
                if(IsKeyPressed(KEY_ENTER))
                {
                    if(IsKeyPressed(KEY_ENTER))
                    {
                        PlaySound(selecionar);
                        EndDrawing();
                        Controles(som, *musica, textura, introducao, selecionar); //Abre a tela de controles
                    }
                }
                break;
            case 3://Opção dificuldade
                DrawRectangle(LARGURA / 8, ALTURA * 4 / 10, LARGURA * 6 / 8, ALTURA / 12, DARKGREEN);
                if(IsKeyPressed(KEY_ENTER))
                {
                    if(IsKeyPressed(KEY_ENTER))
                    {
                        PlaySound(selecionar);
                        if(*dificuldade == 1) //Caso a dificuldade seja fácil
                        {
                            *dificuldade = 2; //Dificuldade dificil (velocidade dos monstros dobrada e vida = 1)
                        }
                        else if(*dificuldade == 2) //Caso a dificuldade seja dificil
                        {
                            *dificuldade = 1; //Dificuldade facil
                        }
                        (*vida)--;
                    }
                }
                break;
            case 4://Opção mutar/desmutar musica
                DrawRectangle(LARGURA / 8, ALTURA * 5.5 / 10, LARGURA * 6 / 8, ALTURA / 12, DARKGREEN);
                if(IsKeyPressed(KEY_ENTER))
                {
                    PlaySound(selecionar);
                    if(*musica == 1) //Caso o jogo esteja desmutado
                    {
                        *musica = 0; //Muta o jogo
                    }
                    else if(*musica == 0) //Caso o jogo esteja mutado
                    {
                        *musica = 1; //Desmuta o jogo
                    }
                }
                break;
            case 5: //Modo Deus
                DrawRectangle(LARGURA / 8, ALTURA * 7 / 10, LARGURA * 6 / 8, ALTURA / 12, DARKGREEN);
                if(IsKeyPressed(KEY_ENTER))
                {
                    PlaySound(selecionar);
                    if(*modoDeus == 0)
                    {
                        *modoDeus = 1;
                    }
                    else
                    {
                        *modoDeus = 0;
                    }
                }
                break;

            case 6: //Formatar saves e placar
                DrawRectangle(LARGURA / 8, ALTURA * 8.5 / 10, LARGURA * 6 / 8, ALTURA / 12, DARKGREEN);
                if(IsKeyPressed(KEY_ENTER))
                {
                    PlaySound(selecionar);
					//Remove os 3 saves
                    remove("save1");
                    remove("save2");
                    remove("save3");
					//Ainda não existe placar
                }
                break;
        }
		//Textos das opções
        DrawText("Voltar", LARGURA / 2 - (MeasureText("Voltar", 60) / 2), ALTURA * 1 / 10, 60, WHITE);
        DrawText("Controles", LARGURA / 2 - (MeasureText("Controles", 60) / 2), ALTURA * 2.5 / 10, 60, WHITE);
        DrawText("Dificuldade", LARGURA / 2 - (MeasureText("Dificuldade", 60) / 2), ALTURA * 4 / 10, 60, WHITE);
        DrawText("Mutar/Desmutar Musica", LARGURA / 2 - (MeasureText("Mutar/Desmutar Musica", 60) / 2), ALTURA * 5.5 / 10, 60, WHITE);
        DrawText("Modo Deus", LARGURA / 2 - (MeasureText("Modo Deus", 60) / 2), ALTURA * 7 / 10, 60, WHITE);
        DrawText("Formatar Saves e Placar", LARGURA / 2 - (MeasureText("Formatar Saves e Placar", 60) / 2), ALTURA * 8.5 / 10, 60, WHITE);
        DrawText("Aviso! Mudar a dificuldade reinicia o mapa!", LARGURA / 2 - (MeasureText("Aviso! Mudar a dificuldade reinicia o mapa!", 30) / 2), ALTURA * 9.5 / 10, 30, RED);
        if(IsKeyPressed(KEY_ESCAPE))
        {
            config = 1; //Quebra o laço ao apertar esc
        }
        SelecionadorMenus(&opcaoConfig, 6); //Permite a seleção das opções com WASD ou as setas
        EndDrawing();
    }
}
//Função ZeraObjetos
//Torna os atributos de um objeto zero para o iniciar ou quando ele se colidir com algo
void ZeraObjetos(Rectangle *objetos, int *objetosArremessados, char *sentido)
{
    objetos->height = 0;
    objetos->width = 0;
    objetos->x = 0;
    objetos->y = 0;
    *objetosArremessados -= 1;
    *sentido = 's';
}

//Função ZeraMonstros
//Coloca as posições de monstro em 0 para o iniciar ou o matar
void ZeraMonstros(Rectangle *monstro)
{
    monstro->height = 0;
    monstro->width = 0;
    monstro->x = 0;
    monstro->y = 0;
}
//Função ColetaColetavel
//Aumenta a contagem de coletáveis e aumenta a pontuação
void ColetaColetavel(int *coletaveis, int *pontuacaoFaseAtual,  int *pontuacao, Sound coletarColetavel)
{
    int j;
    *coletaveis += 1;
    j = GetRandomValue(25, 50);
    *pontuacao += j;
    *pontuacaoFaseAtual += j;
    PlaySound(coletarColetavel);
}
//Função main
int main(void)
{//Declarações
    int fechar = 0, musica = 1, opcao = 1, pause, iniciou = 0, continuarMenu = 0, monstrosNoMapa, contadorTextura;
    int partida, comeco, lerMapa, opcaoContinuar, opcaoPause, numeroDeMapas = 0, dificuldade = 1;
	int j, k = 0;
	char mapaAtual[20], mapaFundo[30];
	char posicoesDaTela[LINHAS][COLUNAS];
    char nomeDoJogo[40] = {"A Jornada da Freira"};
	JOGADOR jogador;
	MONSTRO monstros[MAX_MONSTROS];
	SAVES saves;
	PORTAL portal;
	RELIQUIA reliquia;
	FILE *mapa;
	FILE *savesJogo;
    //--------------------------------------------------------------------------------------
    //Inicializações
    //--------------------------------------------------------------------------------------
    InitWindow(LARGURA, ALTURA, "A Jornada da Freira");//Inicializa janela, com certo tamanho e título
    InitAudioDevice();//Inicia dispostivi de som
    SetTargetFPS(60);// Ajusta a execução do jogo para 60 frames por segundo
	//Carregando texturas
	Texture fundo;
    Texture controles = LoadTexture("./images/controles.png");
    Texture save = LoadTexture("./images/save.png");
    Texture seta = LoadTexture("./images/seta.png");
    Texture blocoNormal = LoadTexture("./images/blocoNormal.png");
    Texture blocoQuebravel = LoadTexture("./images/blocoQuebravel.png");
    Texture blocoEspinho = LoadTexture("./images/blocoEspinho.png");
    Texture coletavel = LoadTexture("./images/coletavel.png");
    Texture pontos = LoadTexture("./images/pontos.png");
    Texture vida = LoadTexture("./images/vida.png");
    Texture monstro1 = LoadTexture("./images/monstro1.png");
    Texture gameOver = LoadTexture("./images/gameOver.png");
    Texture levelComplete = LoadTexture("./images/levelComplete.png");
    Texture finalBom = LoadTexture("./images/finalBom.png");
    Texture finalRuim = LoadTexture("./images/finalRuim.png");
    jogador.skin = LoadTexture("./images/jogador.png");
    portal.portal = LoadTexture("./images/portal.png");
    reliquia.reliquia = LoadTexture("./images/Reliquia.png");
    jogador.objeto = LoadTexture("./images/objeto.png");
    jogador.soco = LoadTexture("./images/soco.png");
	//Carregando musicas
    Music crumbling_castle = LoadMusicStream("./music/crumbling_castle.mp3");
    Music intrasport = LoadMusicStream("./music/intrasport.mp3");
	//Carregando efeitos sonoros
    Sound selecionar = LoadSound("./sfx/selecionar.mp3");
    Sound introducao = LoadSound("./sfx/introdução.mp3");
    Sound novoJogo = LoadSound("./sfx/novoJogo.mp3");
    Sound continuar = LoadSound("./sfx/continuar.mp3");
    Sound config = LoadSound("./sfx/config.mp3");
    Sound skins = LoadSound("./sfx/skins.mp3");
    Sound placar = LoadSound("./sfx/placar.mp3");
    Sound sair = LoadSound("./sfx/sair.mp3");
    Sound voltar = LoadSound("./sfx/voltar.mp3");
    Sound falhaAbrirMapa = LoadSound("./sfx/falhaAbrirMapa.mp3");
    Sound gameOver1 = LoadSound("./sfx/gameOver1.mp3");
    Sound gameOver2 = LoadSound("./sfx/gameOver2.mp3");
    Sound somLevelComplete = LoadSound("./sfx/levelComplete.mp3");
    Sound coletarReliquia = LoadSound("./sfx/coletarReliquia.mp3");
    Sound coletarColetavel = LoadSound("./sfx/coletarColetavel.mp3");
    Sound fimBom = LoadSound("./sfx/fimBom.mp3");
    Sound fimRuim = LoadSound("./sfx/fimRuim.mp3");
    jogador.socar = LoadSound("./sfx/soco.mp3");
    jogador.arramessarObjeto = LoadSound("./sfx/arremessarObjeto.mp3");
    jogador.falhaObjeto = LoadSound("./sfx/falhaObjeto.mp3");
    jogador.morte = LoadSound("./sfx/jogadorMorte.mp3");
    Sound monstrosMorte = LoadSound("./sfx/monstrosMorte.mp3");
    Sound quebrarBlocos = LoadSound("./sfx/quebrarBlocos.mp3");
    saves.selec1 = LoadSound("./sfx/selecSave1.mp3");
    saves.selec2 = LoadSound("./sfx/selecSave2.mp3");
    saves.selec3 = LoadSound("./sfx/selecSave3.mp3");
	//Iniciação de alguns atributos com constantes
    jogador.velocidadePulo = VELOCIDADE_PULO;
    jogador.corpo.width = QUADRADO;
    jogador.corpo.height = QUADRADO;
    jogador.velocidade = VELOCIDADE_MOVIMENTO;
    jogador.modoDeus = 0;
    SetRandomSeed(time(NULL));
    //Contagem de mapas
    for(j = 1; k == 0; j++)//Conta quantos mapas existem até o mapa segHUDnte nao ser aberto
    {
        sprintf(mapaAtual, "mapa%d.txt\0", j);//Vai montando strings com o nome do mapa com cada valor de J
        if(!(mapa = fopen(mapaAtual, "r")))//Caso não seja possível ler o mapa
        {
            k = 1;//Determina que o último mapa foi contador
        }
        else
        {
            numeroDeMapas++;//Mais um mapa é contador
            fclose(mapa);//Mapa fechado
        }
    }
    //Inicia a textura dos saves para posterior exibição
    for(j = 0; j < NUMERO_SAVES; j++)
    {
        saves.textura[j].x = 0;
        saves.textura[j].y = 0;
        saves.textura[j].width = 300;
        saves.textura[j].height = 300;
    }
    //--------------------------------------------------------------------------------------
    //Laço Do Jogo
    //--------------------------------------------------------------------------------------
    while (fechar == 0) //O jogo só irá se fechar quando a variável fechar for diferente de 0. Isso só occore clicando em sair no menu principal.
    {
        //--------------------------------------------------------------------------------------
        //Menu Principal
        //--------------------------------------------------------------------------------------
        if(iniciou == 0) //Tela de controles, exibida uma única vez ao iniciar o jogo
        {
            iniciou = Controles(intrasport, musica, controles, introducao, selecionar);
        }
        BeginDrawing();
        ClearBackground(BLACK);
		//Titulo
        DrawRectangle(LARGURA / 2 - (MeasureText(nomeDoJogo, 80) / 2), ALTURA * 0.25 / 10, MeasureText(nomeDoJogo, 80), ALTURA / 10, BLUE);
        DrawText(nomeDoJogo, (LARGURA / 2) - (MeasureText(nomeDoJogo, 80) / 2), ALTURA * 0.25 / 10 , 80, WHITE);
        //Retangulos das opções
        DrawRectangle(LARGURA / 8, ALTURA * 1.75 / 10, LARGURA * 6 / 8, ALTURA / 12, RED);
        DrawRectangle(LARGURA / 8 , ALTURA * 3.25 / 10, LARGURA * 6 / 8, ALTURA / 12, RED);
        DrawRectangle(LARGURA / 8, ALTURA * 4.75 / 10, LARGURA * 6 / 8, ALTURA / 12, RED);
        DrawRectangle(LARGURA / 8, ALTURA * 6.25 / 10, LARGURA * 6 / 8, ALTURA / 12, RED);
        DrawRectangle(LARGURA / 8, ALTURA * 7.75 / 10, LARGURA * 6 / 8, ALTURA / 12, RED);
        DrawRectangle(LARGURA / 8, ALTURA * 9.25 / 10, LARGURA * 6 / 8, ALTURA / 12, RED);

        opcaoContinuar = 0; //Deixa a opção como 0 para caso a opção selecionada seja Novo Jogo
        //--------------------------------------------------------------------------------------
        //Opcções do Menu Principal
        //--------------------------------------------------------------------------------------
        switch(opcao) //Retangulos são desenhados por cima dos originais pra mostrar a opção selecionada
        {
             //--------------------------------------------------------------------------------------
             //Continuar Jogo
             //--------------------------------------------------------------------------------------
            case 2:
                DrawRectangle(LARGURA / 8, ALTURA * 3.25 / 10, LARGURA * 6 / 8, ALTURA / 12, (Color){80, 3, 6, 255});
                if(IsKeyPressed(KEY_ENTER))
                {
                    PlaySound(selecionar);
                    PlaySound(continuar);
                    opcaoContinuar = 1; //Faz com quem os arquivos sejam tirados do save e não do mapa
                }
                if(opcaoContinuar == 0)
                {
                    break;
                }
            //--------------------------------------------------------------------------------------
            //Novo Jogo
            //--------------------------------------------------------------------------------------
            case 1:
                DrawRectangle(LARGURA / 8, ALTURA * 1.75 / 10, LARGURA * 6 / 8, ALTURA / 12, (Color){80, 3, 6, 255});
                if(IsKeyPressed(KEY_ENTER))
                {
                    EndDrawing();
                    if(opcaoContinuar == 0) //Garante que o audio não sera executado para a segunda opção
                    {
                        PlaySound(selecionar);
                        PlaySound(novoJogo);
                    }
                    continuarMenu = 1; //Mantem a seleção de save em loop
                    saves.opcao = 1; //Inicia a opção do save na primeira opção
                    saves.contador = 0; //Conta o tempo entre cada mudança da animação dos saves
                    partida = 0; //Quando 1, continua a inicialização da partida e evita que o save selecionado seja mudado durante o carregamento
                    comeco = 0; //Determina o fim da animação e o começo do jogo em si
                    saves.lido = 0; //Se o save já foi lido ou criado vira 1
					//--------------------------------------------------------------------------------------
                    //Seleção de Save
                    //--------------------------------------------------------------------------------------
                    while(continuarMenu == 1)
                    {

                        Musica(intrasport, musica);
                        BeginDrawing();
                        ClearBackground(BLACK);
						//Textos da tela de seleção de save
                        DrawText("Escolha um save para jogar!", (LARGURA / 2) - (MeasureText("Escolha um save para jogar!", 60) / 2), ALTURA * 0.25 / 10 , 60, RAYWHITE);
                        DrawText("Save 1", (LARGURA / 16) + (DIMENSOES_SAVE / 2) - (MeasureText("Save 1", 30) / 2), ALTURA * 2.6 / 10 , 30, RED);
                        DrawText("Save 2", (LARGURA * 3 / 8) + (DIMENSOES_SAVE / 2) - (MeasureText("Save 2", 30) / 2), ALTURA * 2.6 / 10 , 30, RED);
                        DrawText("Save 3", (LARGURA * 11 / 16) + (DIMENSOES_SAVE / 2) - (MeasureText("Save 3", 30) / 2), ALTURA * 2.6 / 10 , 30, RED);
                        //Desenha as texturas dos retângulos dos saves e o retângulo para retornar
                        DrawTextureRec(save, saves.textura[0], (Vector2){LARGURA / 16, ALTURA * 3.2 / 10}, WHITE);
                        DrawTextureRec(save, saves.textura[1], (Vector2){LARGURA * 3 / 8, ALTURA * 3.2 / 10}, WHITE);
                        DrawTextureRec(save, saves.textura[2], (Vector2){LARGURA * 11 / 16, ALTURA * 3.2 / 10}, WHITE);
                        DrawRectangle(0, ALTURA  * 15 / 16, LARGURA * 7 / 16, ALTURA  * 15 / 16, GRAY);
						//Seletor de opção na tela dos saves
                        if((IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) && saves.opcao < 3 && partida == 0)
                        {
                            saves.opcao++;
                        }
                        else if((IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) && (saves.opcao > 1 && saves.opcao != 4) && partida == 0)
                        {
                            saves.opcao--;
                        }
                        else if((IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) && partida == 0)
                        {
                            saves.opcao = 4; //Opção 4 é voltar e está na parte inferior da tela
                        }
                        else if((IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) && saves.opcao == 4 && partida == 0)
                        {
                            saves.opcao = 2;
                        }
						//Desenha setas apontando para a opção que se pretende abrir ou um retângulo para a opção voltar
                        switch(saves.opcao)
                        {
                            case 1: //Seta para o 1o save
                                DrawTexture(seta, LARGURA * 41 / 240, ALTURA * 6.5 / 10, WHITE);
                                break;

                            case 2://Seta para o 2o save
                                DrawTexture(seta, LARGURA * 29 / 60, ALTURA * 6.5 / 10, WHITE);
                                break;

                            case 3://Seta para o 3o save
                                DrawTexture(seta, LARGURA * 191 / 240, ALTURA * 6.5 / 10, WHITE);
                                break;

                            case 4://Retângulo de cor diferente
                                DrawRectangle(0, ALTURA  * 15 / 16, LARGURA * 7 / 16, ALTURA  * 15 / 16, RAYWHITE);
                                break;

                        }
                        DrawText("Selecione para voltar", 10, ALTURA  * 15 / 16, 40, GOLD); //Texto da opção de voltar
						//Opção de save selecionada
                        if((IsKeyPressed(KEY_ENTER) || partida == 1) && saves.opcao != 4) //Executa se a opção não for a 3 (que é o retorno) e continua em execução caso tenha sido selecionada pela primeira vez com enter
                        {
                            partida = 1;//Para condição continue e a animação termine
                            PlaySound(selecionar);
							if(saves.lido == 0) //Executa se os saves ainda não foram lidos
                            {
                                        sprintf(saves.atual, "save%d.txt\0", saves.opcao);//Determina o nome do save a ser aberto ou excluido com base na opção de save
                                        switch(saves.opcao) //Audio dizendo o save aberto
                                        {
                                            case 1:
                                                PlaySound(saves.selec1);
                                                break;
                                            case 2:
                                                PlaySound(saves.selec2);
                                                break;
                                            case 3:
                                                PlaySound(saves.selec3);
                                                break;
                                        }
										//ABRINDO OS SAVES
                                        if(opcaoContinuar == 1) //Caso a opção seja continuar jogo
                                        {
                                            if(!(savesJogo = fopen(saves.atual, "r")))//Caso o save não tenha sido aberto
                                            {
                                                remove(saves.atual); //Remove qualquer arquivo de save que existir com o numero selecionado
												//Iniciação de um save novo, atributos do jogador iniciados
                                                jogador.vidaTotal = VIDA_TOTAL_INICIAL;
                                                jogador.pontuacao = 0;
                                                jogador.reliquias = 0;
                                                jogador.faseAtual = 1;
                                                jogador.pontuacao = 0;
                                                jogador.vida = 0; //Começa em 0 pra garantir a leitura adequada do mapa posteriormente
                                            }
                                            else //Caso ele tenha sido aberto
                                            {
                                                for(j = 0; j < LINHAS; j++)
                                                {
                                                    for(k = 0; k < COLUNAS; k++)
                                                    {
                                                        posicoesDaTela[j][k] = fgetc(savesJogo); //Lê cada posição do mapa do arquivo
                                                    }
                                                }
                                                fscanf(savesJogo, "%d %d %d %d %d", &monstrosNoMapa, &dificuldade, &musica, &contadorTextura, &jogador.modoDeus); //Lê algumas outras informações sobre configurações e sobre o mapa
                                                for(j = 0; j < MAX_MONSTROS; j++)
                                                {//Lê todos os detalhes de cada monstro
                                                    fscanf(savesJogo, "%d %d %f %f %f %f\n", &monstros[j].sentido, &monstros[j].contador, &monstros[j].corpo.x, &monstros[j].corpo.y, &monstros[j].corpo.width, &monstros[j].corpo.height);
                                                    monstros[j].velocidade = VELOCIDADE_MONSTROS_PADRAO * dificuldade; //A velocidade é constante mas depende da dificuldade
                                                }
												//Lê todas informações relacionadas ao jogador
                                                fscanf(savesJogo, "%f %f %c %d %d %d %d %d %d %d %d %d %d %d\n", &jogador.corpo.x, &jogador.corpo.y, &jogador.sentido, &jogador.vidaTotal, &jogador.puloAux, &jogador.coletaveis, &jogador.pontuacao, &jogador.pontuacaoFaseAtual, &jogador.reliquias, &jogador.reliquiaFaseAtual, &jogador.faseAtual, &jogador.objetosArremessados, &jogador.direcao, &jogador.contador);
                                                for(j = 0; j < MAX_OBJETOS; j++)
                                                {//Lê informações relacionadas aos objetos do jogador
                                                    fscanf(savesJogo, "%f %f %f %f %c\n", &jogador.objetos[j].x, &jogador.objetos[j].y, &jogador.objetos[j].width, &jogador.objetos[j].height, &jogador.sentidoObjetos[j]);
                                                }
                                                fscanf(savesJogo, "%f %f %d\n", &portal.pos.x, &portal.pos.y, &portal.contador); //Lê informações do portal
                                                fscanf(savesJogo, "%f %f %f %f\n", &reliquia.pos.x, &reliquia.pos.y, &reliquia.pos.width, &reliquia.pos.height); //Lê informações da reliquia
                                                jogador.vida = 1; //Permite que o loop de gameplay ocorra e evita que o mapa seja lido
                                                lerMapa = 1; //Também ajuda a evitar que o mapa seja lido
                                                sprintf(mapaFundo, "./images/fundo%d.png", jogador.faseAtual);//Determina qual imagem de fundo deve ser carregada
                                                UnloadTexture(fundo);//Limpa qualquer fundo já carregado
                                                fundo = LoadTexture(mapaFundo); //Carrega o fundo desejado
                                                if(fundo.id <= 0) //Caso nenhum fundo tenha sido carregado
                                                {
                                                    fundo = LoadTexture("./images/fundo1.png"); //Carrega o fundo1 para não ficar sem fundo
                                                }
                                                fclose(savesJogo);
                                            }
                                        }
                                        else
                                        {
                                            remove(saves.atual); //Remove qualquer arquivo de save que existir com o numero selecionado
                                            //Iniciação de um save novo, atributos do jogador iniciados
                                            jogador.vidaTotal = VIDA_TOTAL_INICIAL;
                                            jogador.pontuacao = 0;
                                            jogador.reliquias = 0;
                                            jogador.faseAtual = 1;
                                            jogador.pontuacao = 0;
                                            jogador.vida = 0; //Começa em 0 pra garantir a leitura adequada do mapa posteriormente
                                        }
                                        sprintf(mapaAtual, "mapa%d.txt\0", jogador.faseAtual); //Define o mapa atual como o informado na faseAtual
                                        saves.lido = 1; //Save já foi lido

                            }
                            saves.contador++; //Atualiza o contador pra animação da textura
                            if(saves.contador == 10) //Atualiza a textura a cada dez frames
                            {
                                saves.contador = 0; //Reinicia a contagem
                                saves.textura[saves.opcao - 1].x += 300; //Atualiza a textura do respectivo save
                                comeco++; //Conta quantos quadros da animação já foram mostrados
                            }
							//--------------------------------------------------------------------------------------
                            //Loop da gameplay
                            //--------------------------------------------------------------------------------------
                            while(comeco == 6) //Executa quando o último quadro da animação for mostrado
                            {
                                StopMusicStream(intrasport);
                                if(jogador.vida == 0) //Só executa enquanto a fase não for reiniciada(ou uma nova iniciada) e o save não tiver sido lido
                                {
                                    lerMapa = 0; //Garante que o mapa seja lido apenas uma vez
                                }
                                jogador.vida = 1; //Define o número de ataques que o jogador pode tomar antes de reiniciar a fase como 1
                                while(jogador.vida > 0)
                                {
                                    if(lerMapa == 0) //Testa se o mapa ainda não foi lido
                                    {
                                        if(!(mapa = fopen(mapaAtual, "r"))) //Abre o mapa, e caso falhe
                                        {
                                            PlaySound(falhaAbrirMapa);
                                            comeco = 0; //Sai do loop de gameplay
                                            continuarMenu = 0; //Sai do menu de saves
                                        }
                                        else //Caso funcione
                                        {
                                            monstrosNoMapa = 0;//Contagem de monstros começa como 0
                                            for(j = 0; j < LINHAS; j++)
                                            {
                                                for(k = 0; k < COLUNAS + 1; k++) //Passa pelos \n no fim de cada linha
                                                {
                                                    if(k < COLUNAS) //Caso o valor seja qualquer um menos \n
                                                    {
                                                        posicoesDaTela[j][k] = fgetc(mapa); //Preenche a matriz de posições na tela com os caracteres de cada posição do mapa
		                                                if(posicoesDaTela[j][k] == 'Z') //Caso a posição seja um Monstro
														{
															//Tamanho e coordenadas do monstro iniciadas
															monstros[monstrosNoMapa].corpo.width = QUADRADO;
															monstros[monstrosNoMapa].corpo.height = QUADRADO;
															monstros[monstrosNoMapa].corpo.x = k * QUADRADO;//Posição x iniciada
															monstros[monstrosNoMapa].corpo.y =  HUD + (j * QUADRADO); //Posição y iniciada, HUD é somado devido ao HUD na parte de cima da tela
															monstros[monstrosNoMapa].velocidade = VELOCIDADE_MONSTROS_PADRAO * dificuldade; //Inicia a velocidade dos monstros
															monstros[monstrosNoMapa].contador = MONSTROS_NOVO_SENTIDO; //O contador para uma nova direção do monstro começa no com o valor que define um novo sentido para que o sentido inicial seja aleatório
															monstrosNoMapa++; //Um monstro extra é adicionado a contagem
															posicoesDaTela[j][k] = ' '; //A posição pois não é mais necessária
														}
														else if(posicoesDaTela[j][k] == 'J')//Caso a posição seja o Jogador
														{
															jogador.corpo.x = k * QUADRADO; //Posição x iniciada
															jogador.corpo.y = HUD + (j * QUADRADO); //Posição y iniciada, HUD é somado devido ao HUD na parte de cima da tela
															posicoesDaTela[j][k] = ' '; //A posição pois não é mais necessária
														}
														else if(posicoesDaTela[j][k] == 'P')//Caso a posição seja o portal
														{
															portal.pos.x = k * QUADRADO; //Posição x iniciada
															portal.pos.y = HUD + (j * QUADRADO); //Posição y iniciada, HUD é somado devido ao HUD na parte de cima da tela
															posicoesDaTela[j][k] = ' '; //A posição pois não é mais necessária
														}
														else if(posicoesDaTela[j][k] == 'H')//Caso a posição seja a reliquia
														{
															reliquia.pos.x = k * QUADRADO; //Posição x iniciada
															reliquia.pos.y = HUD + (j * QUADRADO); //Posição y iniciada, HUD é somado devido ao HUD na parte de cima da tela
															reliquia.pos.height = QUADRADO;
															reliquia.pos.width = QUADRADO;
															posicoesDaTela[j][k] = ' '; //A posição pois não é mais necessária
														}
                                                    }
                                                    else //Isso significa que a linha acabou e o próximo valor é um \n, que deve ser ignorado
                                                    {
                                                        fgetc(mapa); //Não armazena \n
                                                    }
                                                }
                                            }
                                            for(j = monstrosNoMapa; j  < MAX_MONSTROS; j++)
                                            {//Zera todos as variáveis dos outros monstros inexistentes para que não atrapalhem
										        ZeraMonstros(&monstros[j].corpo);
                                                monstros[j].sentido = 0;
                                                monstros[j].contador = 0;
                                            }
                                            lerMapa = 1; //Evita que o  mapa seja lido mais de uma vez
                                            fclose(mapa); //Fecha arquvio após leitura
                                            jogador.puloAux = 0;  //Atua na gravidade, para que seja aplicada apenas enquanto não está pulando
                                            jogador.pulo = 0; //Evita que o jogador pule antes de atingir solo
                                            jogador.puloMax = 0; //Mostra que a altura maxima de pulo não foi alcançada ainda
                                            jogador.sentido = 'd'; //O jogador começa virado para a direita
                                            jogador.coletaveis = 0; //Inicia o total de coletáveis em 0
                                            jogador.pontuacaoFaseAtual = 0; //A pontuação adquiadqHUDridarida na fase atual começa em 0 a cada morte ou novo começo
                                            jogador.reliquiaFaseAtual = 0; //Determina se a reliquia da fase atual foi coletada
                                            jogador.objetosArremessados = MAX_OBJETOS; //Contagem de objetos começa em MAX_OBJETOS pois a função posterior o diminui por este valor
                                            portal.contador = 0; //Começa a animação do portal em 0
                                            jogador.contador = 0; //Contador da animação do jogador começa em 0
                                            jogador.direcao =  -1;//Direção da textura do jogador começa para direita
                                            contadorTextura = 0; //Inicia a textura no 0
											sprintf(mapaFundo, "./images/fundo%d.png", jogador.faseAtual);//Determina qual imagem de fundo deve ser carregada
											UnloadTexture(fundo);//Limpa qualquer fundo já carregado
											fundo = LoadTexture(mapaFundo); //Carrega o fundo desejado
											if(fundo.id <= 0) //Caso nenhum fundo tenha sido carregado
											{
												fundo = LoadTexture("./images/fundo1.png"); //Carrega o fundo1 para não ficar sem fundo
											}
                                            for(j = 0; j < MAX_OBJETOS; j++)
                                            {
                                                ZeraObjetos(&jogador.objetos[j], &jogador.objetosArremessados, &jogador.sentidoObjetos[j]);
                                            }
                                        }
                                    }
                                    BeginDrawing();
                                    //Desenho do fundo
                                    DrawTexture(fundo, 0, HUD, WHITE);
                                    //Desenho de cada espaço na tela
                                    for(j = 0; j < LINHAS; j++)
                                    {
                                        for(k = 0; k < COLUNAS; k++)
                                        {
                                            if(posicoesDaTela[j][k] == '#') //Desenha as parades
                                            {
                                                DrawTexture(blocoNormal, k * QUADRADO, HUD + j * QUADRADO, WHITE);
                                            }
                                            else if(posicoesDaTela[j][k] == 'B') //Desenha os blocos quebráveis
                                            {
                                                DrawTexture(blocoQuebravel, k * QUADRADO, HUD + j * QUADRADO, WHITE);
                                            }
                                            else if(posicoesDaTela[j][k] == 'C') //Desenha os coletáveis
                                            {
                                                DrawTexture(coletavel, k * QUADRADO, HUD + j * QUADRADO, WHITE);
                                            }
                                            else if(posicoesDaTela[j][k] == 'X') //Desenha os espinhos
                                            {
                                                DrawTexture(blocoEspinho, k * QUADRADO, HUD + j * QUADRADO, WHITE);
                                            }

                                        }
                                    }
									//Desenhos da reliquia e do portal
                                    DrawTexture(reliquia.reliquia, reliquia.pos.x, reliquia.pos.y, WHITE);
                                    DrawTextureRec(portal.portal, (Rectangle){20 * (portal.contador / 5), 0, QUADRADO, QUADRADO}, portal.pos, WHITE);
									//Atualiza a textura de portal
                                    portal.contador++;
                                    contadorTextura++;
                                    //Monstros
                                    for(j = 0; j < monstrosNoMapa; j++)//Passa por cada um dos monstros no mapa atual
                                    {
                                        DrawTextureRec(monstro1, (Rectangle){QUADRADO * (contadorTextura / 10), 0, monstros[j].corpo.width * monstros[j].sentido, monstros[j].corpo.height}, (Vector2){monstros[j].corpo.x, monstros[j].corpo.y}, WHITE);//Desenha os monstros
                                        MoveMonstros(&monstros[j], posicoesDaTela, MONSTROS_NOVO_SENTIDO); //Move os monstros
                                        if(CheckCollisionRecs(jogador.corpo, monstros[j].corpo))//Colisão com os monstros
                                        {
                                            jogador.vida--; //Reinicia a fase
                                            if(dificuldade == 1)
                                            {
                                                jogador.vidaTotal--; //Dificuldade facil diminui uma vida
                                            }
                                            else
                                            {
                                                jogador.vidaTotal = 0; //Dificuldade dificil game over instantâneo
                                            }
                                            jogador.pontuacao += (PONTOS_MORTE - jogador.pontuacaoFaseAtual); //Pontuação é voltada ao valor do inicio da fase e tirado uma constante
                                            if(jogador.reliquiaFaseAtual == 1) //Caso a reliquia da fase tenha sido pegada, retira ela
                                            {
                                                jogador.reliquias--;
                                            }
                                            PlaySound(jogador.morte);
                                        }
                                        for(k = 0; k < MAX_OBJETOS; k++)
                                        {
                                            if(CheckCollisionRecs(jogador.objetos[k], monstros[j].corpo)) //Caso o monstro colida com um objeto arremessado pelo jogador
                                            {//Zera tanto o monstro como o objeto e acrescenta a pontuação do jogador
                                                ZeraMonstros(&monstros[j].corpo);
                                                jogador.pontuacao += PONTOS_MONSTRO;
                                                jogador.pontuacaoFaseAtual += PONTOS_MONSTRO;
                                                ZeraObjetos(&jogador.objetos[k], &jogador.objetosArremessados, &jogador.sentidoObjetos[k]);
                                                PlaySound(monstrosMorte);
                                            }
                                        }

                                    }

                                    //Movimentos do jogador
                                    if((int)jogador.corpo.y % QUADRADO != 0)
                                    {//Testa se o jogador.corpo.y está entre 2 quadrados
                                        if((IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) && (posicoesDaTela[((int)jogador.corpo.y - HUD) / QUADRADO][((int)jogador.corpo.x + QUADRADO) / QUADRADO] != '#' && posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][((int)jogador.corpo.x + QUADRADO) / QUADRADO] != '#') && (posicoesDaTela[((int)jogador.corpo.y - HUD) / QUADRADO][((int)jogador.corpo.x + QUADRADO) / QUADRADO] != 'B' && posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][((int)jogador.corpo.x + QUADRADO) / QUADRADO] != 'B'))
                                        {//Verifica se a posição a direita dos 2 quadrados que o jogador está entre é um obstáculo
                                            jogador.corpo.x += jogador.velocidade; //Move para a direita
                                        }
                                        else if((IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) && (posicoesDaTela[((int)jogador.corpo.y - HUD) / QUADRADO][((int)jogador.corpo.x - jogador.velocidade) / QUADRADO] != '#' && posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][((int)jogador.corpo.x - jogador.velocidade) / QUADRADO] != '#') && (posicoesDaTela[((int)jogador.corpo.y - HUD) / QUADRADO][((int)jogador.corpo.x - jogador.velocidade) / QUADRADO] != 'B' && posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][((int)jogador.corpo.x - jogador.velocidade) / QUADRADO] != 'B'))
                                        {//Verifica se a posição a esquerda dos 2 quadrados que o jogador está entre é um obstáculo
                                            jogador.corpo.x -= jogador.velocidade; //Move para a esquerda
                                        }
                                    }
                                    else//Como é divisível por 20, só é necessário testar um quadrado
                                    {
                                        if((IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) && (posicoesDaTela[((int)jogador.corpo.y - HUD) / QUADRADO][((int)jogador.corpo.x + QUADRADO) / QUADRADO] != '#') && (posicoesDaTela[((int)jogador.corpo.y - HUD) / QUADRADO][((int)jogador.corpo.x + QUADRADO) / QUADRADO] != 'B'))
                                        {//Verifica se a posição a direita do quadrado que o jogador está é um obstáculo
                                            jogador.corpo.x += jogador.velocidade; //Move para a direita
                                        }
                                        else if((IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) && (posicoesDaTela[((int)jogador.corpo.y - HUD) / QUADRADO][((int)jogador.corpo.x - jogador.velocidade) / QUADRADO] != '#') && (posicoesDaTela[((int)jogador.corpo.y - HUD) / QUADRADO][((int)jogador.corpo.x - jogador.velocidade) / QUADRADO] != 'B'))
                                        {//Verifica se a posição a esquerda do quadrado que o jogador está é um obstáculo
                                            jogador.corpo.x -= jogador.velocidade; //Move para a esquerda
                                        }
                                    }
                                    //Virar para lados
                                    if(IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))
                                    {
                                        jogador.contador++;
                                        jogador.sentido = 'b'; //O jogador olha para baixo
                                    }
                                    else if(IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
                                    {
                                        jogador.contador++;
                                        jogador.sentido = 'd'; //Vira-se para direita
                                        jogador.direcao =  -1;
                                    }
                                    else if(IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
                                    {
                                        jogador.contador++;
                                        jogador.sentido = 'e'; //Vira-se para esquerda
                                        jogador.direcao = 1;
                                    }

                                    //Gravidade
                                    if(jogador.puloAux == 0 && (int)jogador.corpo.x % QUADRADO != 0)
                                    {//puloAux diz se a gravidade pode continuar e testa se jogador.corpo.x está entre 2 quadrados
                                        if(posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][(int)jogador.corpo.x / QUADRADO] != '#' && posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][((int)jogador.corpo.x + QUADRADO) / QUADRADO] != '#' && posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][(int)jogador.corpo.x / QUADRADO] != 'B' && posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][((int)jogador.corpo.x + QUADRADO) / QUADRADO] != 'B')
                                        {//Verifica se a posição abaixo dos 2 quadrados que o jogador está entre é um obstáculo
                                            jogador.corpo.y += GRAVIDADE; //Move para baixo
                                            jogador.pulo = 0; //Não permite continuar o pulo
                                        }
                                        else
                                        {
                                            jogador.pulo = 1;//Permite continuar o pulo
                                            jogador.puloAux = 0; //Permite a gravidade continuar
                                        }

                                    }
                                    else if(jogador.puloAux == 0 && posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][(int)jogador.corpo.x / QUADRADO] != '#' && posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][(int)jogador.corpo.x / QUADRADO] != 'B') //Como é divisível por 20, só é necessário testar um quadrado
                                    {//Verifica se a posição abaixo do quadrado que o jogador está é um obstáculo
                                        jogador.corpo.y += GRAVIDADE;//Move para baixo
                                        jogador.pulo = 0; //Não permite continuar o pulo
                                    }
                                    else
                                    {
                                            jogador.pulo = 1; //Permite continuar o pulo
                                            jogador.puloAux = 0; //Permite a gravidade continuar
                                    }

                                    //Pulo
                                    if((IsKeyReleased(KEY_SPACE) || IsKeyReleased(KEY_W) || IsKeyReleased(KEY_UP)) ||  jogador.puloMax >= MAX_PULO)
                                    {//Verifica se a tecla de pulo é largada ou a altura do pulo atingiu seu máximo, finalizando o pulo
                                        jogador.pulo = 0; //Não permite continuar o pulo
                                        jogador.puloMax = 0;  //Reinicia a altura máxima de pulo
                                        jogador.puloAux = 0; //Permite a gravidade continuar
                                    }
                                    else if((int)jogador.corpo.x % QUADRADO != 0)//Caso jogador.corpo.x esteja entre 2 quadrados, é necessário uma verificação com 2 testes
                                    {
                                        if(posicoesDaTela[((int)jogador.corpo.y - HUD - jogador.velocidadePulo) / QUADRADO][(int)jogador.corpo.x / QUADRADO] == '#' || posicoesDaTela[((int)jogador.corpo.y - HUD - jogador.velocidadePulo) / QUADRADO][((int)jogador.corpo.x + QUADRADO) / QUADRADO] == '#' || posicoesDaTela[((int)jogador.corpo.y - HUD - jogador.velocidadePulo) / QUADRADO][(int)jogador.corpo.x / QUADRADO] == 'B' || posicoesDaTela[((int)jogador.corpo.y - HUD - jogador.velocidadePulo) / QUADRADO][((int)jogador.corpo.x + QUADRADO) / QUADRADO] == 'B')
                                        {//Verifica se a posição acima dos 2 quadrados que o jogador está entre é um obstáculo, caso sim encerra o pulo
                                            jogador.pulo = 0; //Não permite continuar o pulo assim que a tecla é largada
                                            jogador.puloMax = 0;  //Reinicia a altura máxima de pulo
                                            jogador.puloAux = 0; //Permite a gravidade continuar
                                        }
                                        else if((IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) && jogador.pulo == 1 && jogador.puloMax <= MAX_PULO)
                                        {//Pula apenas caso o jogador esteja no solo e a altura do pulo esteja menor que o máximo
                                            jogador.corpo.y -= jogador.velocidadePulo; //Faz o jogador ir para cima baseado no valor do pulo por vez, permitindo tamanhos diferentes de pulo
                                            jogador.puloMax += jogador.velocidadePulo; //Aumenta o quanto foi pulado até então, desscontada a gravidade
                                            jogador.puloAux = 1; //Não permite a gravidade continuar
                                        }
                                    }
                                    else if(posicoesDaTela[((int)jogador.corpo.y - HUD - jogador.velocidadePulo) / QUADRADO][(int)jogador.corpo.x / QUADRADO] == '#' || posicoesDaTela[((int)jogador.corpo.y - HUD - jogador.velocidadePulo) / QUADRADO][(int)jogador.corpo.x / QUADRADO] == 'B')
                                    {//Caso jogador.corpo.x seja divisível por QUADRADO, faz apenas um teste, e caso a posição acima do jogador tenha um obstáculo, encerra o pulo
                                            jogador.pulo = 0; //Não permite continuar o pulo assim que a tecla é largada
                                            jogador.puloMax = 0;  //Reinicia a altura máxima de pulo
                                            jogador.puloAux = 0; //Permite a gravidade continuar
                                    }
                                    else if((IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) && jogador.pulo == 1 && jogador.puloMax <= MAX_PULO)
                                    {//Pula apenas caso o jogador esteja no solo e a altura do pulo esteja menor que o máximo
                                        jogador.corpo.y -= jogador.velocidadePulo; //Faz o jogador ir para cima baseado no valor do pulo por vez, permitindo tamanhos diferentes de pulo
                                        jogador.puloMax += jogador.velocidadePulo; //Aumenta o quanto foi pulado até então, desscontada a gravidade
                                        jogador.puloAux = 1; //Não permite a gravidade continuar
                                    }
                                    //Soco
                                    if(IsKeyPressed(KEY_Z))
                                    {
                                        PlaySound(jogador.socar);
                                        switch(jogador.sentido) //O soco vai ser voltado para o sentido que o jogador está
                                        {//Todos os testes servem para garantir que, independente da posição do Jogador, sempre seja contado a área precisa do soco, evitando que blocos sejam "atingidos" mas não quebrados
                                            case 'd'://Caso direita
                                                DrawTextureRec(jogador.soco, (Rectangle){0, 0, -QUADRADO, QUADRADO},(Vector2){jogador.corpo.x + QUADRADO, jogador.corpo.y}, WHITE); //Desenho do soco a direita de Jogador
                                                if(posicoesDaTela[((int)jogador.corpo.y - HUD) / QUADRADO][((int)jogador.corpo.x + QUADRADO) / QUADRADO] == 'B')
                                                {//Teste se a posição a direita do jogador é um bloco quebrável
                                                    posicoesDaTela[((int)jogador.corpo.y - HUD) / QUADRADO][((int)jogador.corpo.x + QUADRADO) / QUADRADO] = 'C'; //Transforma bloco em coletável
                                                    PlaySound(quebrarBlocos);
                                                }
                                                else if((int)jogador.corpo.x % QUADRADO != 0)
                                                {//Caso o jogador esteja entre 2 posições X, testa se a 2 posições a direita é um bloco quebrável
                                                    if(posicoesDaTela[((int)jogador.corpo.y - HUD) / QUADRADO][((int)jogador.corpo.x + (2 * QUADRADO)) / QUADRADO] == 'B')
                                                    {
                                                        posicoesDaTela[((int)jogador.corpo.y - HUD) / QUADRADO][((int)jogador.corpo.x + (2 * QUADRADO)) / QUADRADO] = 'C'; //Transforma bloco em coletável
                                                        PlaySound(quebrarBlocos);
                                                    }
                                                }
                                                if((int)jogador.corpo.y % QUADRADO != 0)
                                                {//Caso o jogador esteja entre 2 posições Y
                                                    if(posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][((int)jogador.corpo.x + QUADRADO) / QUADRADO] == 'B')
                                                    {//Teste se a posição a direita da outra posição Y que Jogador está entre é um bloco quebrável
                                                        posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][((int)jogador.corpo.x + QUADRADO) / QUADRADO] = 'C'; //Transforma bloco em coletável
                                                        PlaySound(quebrarBlocos);
                                                    }
                                                    else if((int)jogador.corpo.x % QUADRADO != 0)
                                                    {//Caso ainda Jogador esteja entre 2 posições X e 2 posições Y, testa a se 2 posições a direita da outra posição Y é quebrável
                                                        if(posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][((int)jogador.corpo.x + (2 * QUADRADO)) / QUADRADO] == 'B')
                                                        {
                                                            posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][((int)jogador.corpo.x + (2 * QUADRADO)) / QUADRADO] = 'C'; //Transforma bloco em coletável
                                                            PlaySound(quebrarBlocos);
                                                        }
                                                    }
                                                }
                                                for(j = 0; j < monstrosNoMapa; j++)//Verifica a colisão com cada um dos monstros
                                                {
                                                    if(CheckCollisionRecs((Rectangle){jogador.corpo.x + QUADRADO, jogador.corpo.y, QUADRADO, QUADRADO}, monstros[j].corpo)) //Teste de colisão com um monstro
                                                    {//Zera a posição e tamanho do monstro e aumenta a pontuação do jogador
														ZeraMonstros(&monstros[j].corpo);
                                                        jogador.pontuacao += PONTOS_MONSTRO; //Pontuação recebe valor
                                                        jogador.pontuacaoFaseAtual += PONTOS_MONSTRO;
                                                        PlaySound(monstrosMorte);
                                                    }
                                                }
                                                break;

                                            case 'e': //Caso esquerda
                                                DrawTexture(jogador.soco, jogador.corpo.x - QUADRADO, jogador.corpo.y, WHITE);//Desenho do soco a esquerda de Jogador
                                                if(posicoesDaTela[((int)jogador.corpo.y - HUD) / QUADRADO][((int)jogador.corpo.x - QUADRADO) / QUADRADO] == 'B')
                                                {//Teste se a posição a esquerda do jogador é um bloco quebrável
                                                    posicoesDaTela[((int)jogador.corpo.y - HUD) / QUADRADO][((int)jogador.corpo.x - QUADRADO) / QUADRADO] = 'C'; //Transforma bloco em coletável
                                                    PlaySound(quebrarBlocos);
                                                }
                                                else if((int)jogador.corpo.x % QUADRADO != 0)
                                                {//Caso o jogador esteja entre 2 posições X, testa se a 2 posições a esquerda é um bloco quebrável
                                                    if(posicoesDaTela[((int)jogador.corpo.y - HUD) / QUADRADO][((int)jogador.corpo.x - (2 * QUADRADO)) / QUADRADO] == 'B')
                                                    {
                                                        posicoesDaTela[((int)jogador.corpo.y - HUD) / QUADRADO][((int)jogador.corpo.x - (2 * QUADRADO)) / QUADRADO] = 'C'; //Transforma bloco em coletável
                                                        PlaySound(quebrarBlocos);
                                                    }
                                                }
                                                if((int)jogador.corpo.y % QUADRADO != 0)
                                                {//Caso o jogador esteja entre 2 posições Y
                                                    if(posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][((int)jogador.corpo.x - QUADRADO) / QUADRADO] == 'B')
                                                    {//Teste se a posição a esquerda da outra posição Y que Jogador está entre é um bloco quebrável
                                                        posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][((int)jogador.corpo.x - QUADRADO) / QUADRADO] = 'C'; //Transforma bloco em coletável
                                                    }
                                                    else if((int)jogador.corpo.x % QUADRADO != 0)
                                                    {//Caso ainda Jogador esteja entre 2 posições X e 2 posições Y, testa a se 2 posições a esquerda da outra posição Y é quebrável
                                                        if(posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][((int)jogador.corpo.x - (2 * QUADRADO)) / QUADRADO] == 'B')
                                                        {
                                                            posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][((int)jogador.corpo.x - (2 * QUADRADO)) / QUADRADO] = 'C'; //Transforma bloco em coletável
                                                            PlaySound(quebrarBlocos);
                                                        }
                                                    }
                                                }
                                                for(j = 0; j < monstrosNoMapa; j++)//Verifica a colisão com cada um dos monstros
                                                {
                                                    if(CheckCollisionRecs((Rectangle){jogador.corpo.x - QUADRADO, jogador.corpo.y, QUADRADO, QUADRADO}, monstros[j].corpo)) //Teste de colisão com um monstro
                                                    {//Zera a posição e tamanho do monstro e aumenta a pontuação do jogador
														ZeraMonstros(&monstros[j].corpo);
                                                        jogador.pontuacao += PONTOS_MONSTRO; //Pontuação recebe valor
                                                        jogador.pontuacaoFaseAtual += PONTOS_MONSTRO;
                                                        PlaySound(monstrosMorte);
                                                    }
                                                }
                                                break;

                                            case 'b': //Caso baixo
                                                DrawTextureRec(jogador.soco, (Rectangle){QUADRADO, 0, QUADRADO, QUADRADO}, (Vector2){jogador.corpo.x, jogador.corpo.y + QUADRADO}, WHITE);//Desenho do soco abaixo de Jogador
                                                if(posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][(int)jogador.corpo.x / QUADRADO] == 'B')
                                                {//Teste se a posição abaixo do jogador é um bloco quebrável
                                                    posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][(int)jogador.corpo.x / QUADRADO] = 'C'; //Transforma bloco em coletável
                                                    PlaySound(quebrarBlocos);
                                                }
                                                else if((int)jogador.corpo.y % QUADRADO != 0)
                                                {//Caso o jogador esteja entre 2 posições Y, testa se 2 posições abaixo é um bloco quebrável
                                                    if(posicoesDaTela[((int)jogador.corpo.y - HUD + (2 * QUADRADO)) / QUADRADO][(int)jogador.corpo.x / QUADRADO] == 'B')
                                                    {
                                                        posicoesDaTela[((int)jogador.corpo.y - HUD + (2 * QUADRADO)) / QUADRADO][(int)jogador.corpo.x / QUADRADO] = 'C'; //Transforma bloco em coletável
                                                        PlaySound(quebrarBlocos);
                                                    }
                                                }
                                                if((int)jogador.corpo.x % QUADRADO != 0)
                                                {//Caso o jogador esteja entre 2 posições X
                                                    if(posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][((int)jogador.corpo.x + QUADRADO) / QUADRADO] == 'B')
                                                    {//Teste se a posição abaixo da outra posição X que Jogador está entre é um bloco quebrável
                                                        posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][((int)jogador.corpo.x + QUADRADO)/ QUADRADO] = 'C'; //Transforma bloco em coletável
                                                        PlaySound(quebrarBlocos);
                                                    }
                                                    else if((int)jogador.corpo.y % QUADRADO != 0)
                                                    {//Caso ainda Jogador esteja entre 2 posições X e 2 posições Y, testa a se 2 posições abaixo da outra posição X é quebrável
                                                        if(posicoesDaTela[((int)jogador.corpo.y - HUD + (2 * QUADRADO)) / QUADRADO][((int)jogador.corpo.x + QUADRADO) / QUADRADO] == 'B')
                                                        {
                                                            posicoesDaTela[((int)jogador.corpo.y - HUD + (2 * QUADRADO)) / QUADRADO][((int)jogador.corpo.x + QUADRADO) / QUADRADO] = 'C'; //Transforma bloco em coletável
                                                            PlaySound(quebrarBlocos);
                                                        }
                                                    }
                                                }
                                                for(j = 0; j < monstrosNoMapa; j++)//Verifica a colisão com cada um dos monstros
                                                {
                                                    if(CheckCollisionRecs((Rectangle){jogador.corpo.x, jogador.corpo.y + QUADRADO, QUADRADO, QUADRADO}, monstros[j].corpo))//Teste de colisão com um monstro
                                                    {//Zera a posição e tamanho do monstro e aumenta a pontuação do jogador
														ZeraMonstros(&monstros[j].corpo);
                                                        jogador.pontuacao += PONTOS_MONSTRO; //Pontuação recebe valor
                                                        jogador.pontuacaoFaseAtual += PONTOS_MONSTRO;
                                                        PlaySound(monstrosMorte);
                                                    }
                                                }
                                                break;

                                        }
                                    }
                                    //Superpoder
                                    if(IsKeyPressed(KEY_X))
                                    {
                                        if((jogador.modoDeus == 1 || jogador.coletaveis >= (COLETAVEIS_PARA_OBJETO * dificuldade)) && jogador.objetosArremessados < MAX_OBJETOS)
                                        {//Testa se o jogador tem coletaveis para usar o poder (ou esta no modo Deus) e se o numero de objetos arremessados atualmente é menor que o maximo
                                            PlaySound(jogador.arramessarObjeto);
                                            jogador.objetosArremessados++; //Começa um novo arremesso
                                            k = 0; //Serve para iniciar as posições do novo arremesso
                                            if(jogador.modoDeus == 0)
                                            {//Se o modo Deus estiver ativado não serão descontados os coletaveis e objetos serão gratuitos
                                                jogador.coletaveis -= COLETAVEIS_PARA_OBJETO * dificuldade; //Desconta coletaveis para o arremesso
                                            }
                                        }
                                        else
                                        {
                                            PlaySound(jogador.falhaObjeto); //Som que indica que não é possível
                                        }
                                    }
                                    if(jogador.objetosArremessados > 0)
                                    {//Caso ao menos um objeto esteja sendo arremessado
                                        for(j = 0; j < MAX_OBJETOS; j++)
                                        {//Teste cada um dos objetos
                                            if(jogador.sentidoObjetos[j] == 's' && k == 0)
                                            {//Testa se ele ainda não tem direção e se k = 0, para então iniciar os objetos
                                                jogador.sentidoObjetos[j] = jogador.sentido; //O sentido do objeto é o mesmo do jogador
                                                switch(jogador.sentidoObjetos[j])
                                                {//Serão iniciadas as posições e dimensões do objeto relativos a posição do jogador para a direção que o jogador está
                                                    case 'd': //Caso direita
                                                        jogador.objetos[j].x = jogador.corpo.x + QUADRADO;
                                                        jogador.objetos[j].y = jogador.corpo.y + (QUADRADO / 4);
                                                        jogador.objetos[j].width = QUADRADO / 2;
                                                        jogador.objetos[j].height = QUADRADO / 2;
                                                        break;
                                                    case 'e': //Caso esquerda
                                                        jogador.objetos[j].x = jogador.corpo.x - (QUADRADO / 2);
                                                        jogador.objetos[j].y = jogador.corpo.y + (QUADRADO / 4);
                                                        jogador.objetos[j].width = QUADRADO / 2;
                                                        jogador.objetos[j].height = QUADRADO / 2;
                                                        break;
                                                    case 'b'://Caso baixo
                                                        jogador.objetos[j].x = jogador.corpo.x + (QUADRADO / 4);
                                                        jogador.objetos[j].y = jogador.corpo.y + QUADRADO;
                                                        jogador.objetos[j].width = QUADRADO / 2;
                                                        jogador.objetos[j].height = QUADRADO / 2;
                                                        break;
                                                    default:
                                                        break;
                                                }
                                                k = 1;
                                            }
                                            if(jogador.objetos[j].y != 0)
                                            {//Se o y do objeto for diferente de 0 significa que ele existe
										        //Desenho do objeto
                                                DrawTextureRec(jogador.objeto, (Rectangle){0, 0, jogador.objetos[j].width, jogador.objetos[j].height}, (Vector2){jogador.objetos[j].x, jogador.objetos[j].y}, WHITE);
                                                switch(jogador.sentidoObjetos[j]) //Atualiza a posição na tela do objeto baseado no sentido e verifica colisões
                                                {
                                                    case 'd': //Caso direita
                                                        jogador.objetos[j].x += VELOCIDADE_OBJETO; //Desloca o objeto com a velocidade dele
                                                        //Testes de colisão com blocos quebráveis, parades e armadilhas
                                                        if(posicoesDaTela[((int)jogador.objetos[j].y - HUD) / QUADRADO][((int)jogador.objetos[j].x + VELOCIDADE_OBJETO) / QUADRADO] == 'B')
                                                        {//Caso a posição for um bloco quebrável
                                                            posicoesDaTela[((int)jogador.objetos[j].y - HUD) / QUADRADO][((int)jogador.objetos[j].x + VELOCIDADE_OBJETO) / QUADRADO] = 'C'; //Substitui por uma moeda
                                                            PlaySound(quebrarBlocos);
                                                            ZeraObjetos(&jogador.objetos[j], &jogador.objetosArremessados, &jogador.sentidoObjetos[j]);//Zera o objeto o fazendo sumir após a colisão
                                                        }
                                                        else if(posicoesDaTela[((int)jogador.objetos[j].y - HUD  + (QUADRADO / 2)) / QUADRADO][((int)jogador.objetos[j].x + VELOCIDADE_OBJETO) / QUADRADO] == 'B')
                                                        {//Caso a posição for um bloco quebrável
                                                            posicoesDaTela[((int)jogador.objetos[j].y - HUD + (QUADRADO / 2)) / QUADRADO][((int)jogador.objetos[j].x + VELOCIDADE_OBJETO) / QUADRADO] = 'C';//Substitui por uma moeda
                                                            PlaySound(quebrarBlocos);
                                                            ZeraObjetos(&jogador.objetos[j], &jogador.objetosArremessados, &jogador.sentidoObjetos[j]);//Zera o objeto o fazendo sumir após a colisão
                                                        }
                                                        else if(posicoesDaTela[((int)jogador.objetos[j].y - HUD) / QUADRADO][((int)jogador.objetos[j].x + VELOCIDADE_OBJETO) / QUADRADO] == '#' ||  posicoesDaTela[((int)jogador.objetos[j].y - HUD) / QUADRADO][((int)jogador.objetos[j].x + VELOCIDADE_OBJETO) / QUADRADO] == 'X' || posicoesDaTela[((int)jogador.objetos[j].y - HUD + (QUADRADO / 2)) / QUADRADO][((int)jogador.objetos[j].x + VELOCIDADE_OBJETO) / QUADRADO] == '#' ||  posicoesDaTela[((int)jogador.objetos[j].y - HUD + (QUADRADO / 2)) / QUADRADO][((int)jogador.objetos[j].x + VELOCIDADE_OBJETO) / QUADRADO] == 'X')
                                                        {
                                                            ZeraObjetos(&jogador.objetos[j], &jogador.objetosArremessados, &jogador.sentidoObjetos[j]);//Zera o objeto o fazendo sumir após a colisão
                                                        }

                                                        break;
                                                    case 'e': //Caso esquerda
                                                        jogador.objetos[j].x -= VELOCIDADE_OBJETO;//Desloca o objeto com a velocidade dele
                                                        //Testes de colisão com blocos quebráveis, parades e armadilhas
                                                        if(posicoesDaTela[((int)jogador.objetos[j].y - HUD) / QUADRADO][((int)jogador.objetos[j].x) / QUADRADO] == 'B')
                                                        {//Caso a posição for um bloco quebrável
                                                            posicoesDaTela[((int)jogador.objetos[j].y - HUD) / QUADRADO][((int)jogador.objetos[j].x) / QUADRADO] = 'C';//Substitui por uma moeda
                                                            PlaySound(quebrarBlocos);
                                                            ZeraObjetos(&jogador.objetos[j], &jogador.objetosArremessados, &jogador.sentidoObjetos[j]);//Zera o objeto o fazendo sumir após a colisão
                                                        }
                                                        else if(posicoesDaTela[((int)jogador.objetos[j].y - HUD  + (QUADRADO / 2)) / QUADRADO][((int)jogador.objetos[j].x) / QUADRADO] == 'B')
                                                        {//Caso a posição for um bloco quebrável
                                                            posicoesDaTela[((int)jogador.objetos[j].y - HUD + (QUADRADO / 2)) / QUADRADO][((int)jogador.objetos[j].x) / QUADRADO] = 'C';//Substitui por uma moeda
                                                            PlaySound(quebrarBlocos);
                                                            ZeraObjetos(&jogador.objetos[j], &jogador.objetosArremessados, &jogador.sentidoObjetos[j]);//Zera o objeto o fazendo sumir após a colisão
                                                        }
                                                        else if(posicoesDaTela[((int)jogador.objetos[j].y - HUD) / QUADRADO][((int)jogador.objetos[j].x) / QUADRADO] == '#' ||  posicoesDaTela[((int)jogador.objetos[j].y - HUD) / QUADRADO][((int)jogador.objetos[j].x) / QUADRADO] == 'X' || posicoesDaTela[((int)jogador.objetos[j].y - HUD + (QUADRADO / 2)) / QUADRADO][((int)jogador.objetos[j].x) / QUADRADO] == '#' ||  posicoesDaTela[((int)jogador.objetos[j].y - HUD + (QUADRADO / 2)) / QUADRADO][((int)jogador.objetos[j].x) / QUADRADO] == 'X')
                                                        {
                                                            ZeraObjetos(&jogador.objetos[j], &jogador.objetosArremessados, &jogador.sentidoObjetos[j]);//Zera o objeto o fazendo sumir após a colisão
                                                        }
                                                        break;
                                                    case 'b': //Caso baixo
                                                        jogador.objetos[j].y += VELOCIDADE_OBJETO;//Desloca o objeto com a velocidade dele
                                                        //Testes de colisão com blocos quebráveis, parades e armadilhas
                                                        if(posicoesDaTela[((int)jogador.objetos[j].y - HUD + VELOCIDADE_OBJETO) / QUADRADO][((int)jogador.objetos[j].x) / QUADRADO] == 'B')
                                                        {//Caso a posição for um bloco quebrável
                                                            posicoesDaTela[((int)jogador.objetos[j].y - HUD  + VELOCIDADE_OBJETO) / QUADRADO][((int)jogador.objetos[j].x) / QUADRADO] = 'C';//Substitui por uma moeda
                                                            PlaySound(quebrarBlocos);
                                                            ZeraObjetos(&jogador.objetos[j], &jogador.objetosArremessados, &jogador.sentidoObjetos[j]);//Zera o objeto o fazendo sumir após a colisão
                                                        }
                                                        else if(posicoesDaTela[((int)jogador.objetos[j].y - HUD + VELOCIDADE_OBJETO) / QUADRADO][((int)jogador.objetos[j].x + (QUADRADO / 2)) / QUADRADO] == 'B')
                                                        {//Caso a posição for um bloco quebrável
                                                            posicoesDaTela[((int)jogador.objetos[j].y - HUD + VELOCIDADE_OBJETO) / QUADRADO][((int)jogador.objetos[j].x + (QUADRADO / 2)) / QUADRADO] = 'C';//Substitui por uma moeda
                                                            PlaySound(quebrarBlocos);
                                                            ZeraObjetos(&jogador.objetos[j], &jogador.objetosArremessados, &jogador.sentidoObjetos[j]);//Zera o objeto o fazendo sumir após a colisão
                                                        }
                                                        else if(posicoesDaTela[((int)jogador.objetos[j].y - HUD + VELOCIDADE_OBJETO) / QUADRADO][((int)jogador.objetos[j].x) / QUADRADO] == '#' ||  posicoesDaTela[((int)jogador.objetos[j].y - HUD + VELOCIDADE_OBJETO) / QUADRADO][((int)jogador.objetos[j].x) / QUADRADO] == 'X' || posicoesDaTela[((int)jogador.objetos[j].y - HUD + VELOCIDADE_OBJETO) / QUADRADO][((int)jogador.objetos[j].x + (QUADRADO / 2)) / QUADRADO] == '#' ||  posicoesDaTela[((int)jogador.objetos[j].y - HUD + VELOCIDADE_OBJETO) / QUADRADO][((int)jogador.objetos[j].x + (QUADRADO / 2)) / QUADRADO] == 'X')
                                                        {
                                                            ZeraObjetos(&jogador.objetos[j], &jogador.objetosArremessados, &jogador.sentidoObjetos[j]);//Zera o objeto o fazendo sumir após a colisão
                                                        }
                                                        break;
                                                    default:
                                                        break;
                                                }
                                            }
                                        }
                                    }
                                    //Espinhos
                                    if((int)jogador.corpo.x % QUADRADO != 0 && (int)jogador.corpo.y % QUADRADO != 0)
                                    {//Caso o jogador esteja entre 2 quadrados em  X e Y
                                        if(posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][((int)jogador.corpo.x + QUADRADO) / QUADRADO] == 'X')
                                        {//Testa se houve colisão com espinhos na posição corrigida do jogador
                                            jogador.vida--; //Reinicia o mapa
                                            if(dificuldade == 1)//Se a dificuldade for fácil perde uma vida
                                            {
                                                jogador.vidaTotal--;
                                            }
                                            else//Se fpr difícil o jogador perde todas vidas imediatamente
                                            {
                                                jogador.vidaTotal = 0;
                                            }
                                            //Pontuação e relíquia na fase reiniciadas e pontuação diminuida por uma taxa adicional
                                            jogador.pontuacao -= (jogador.pontuacaoFaseAtual + 100);
                                            if(jogador.reliquiaFaseAtual == 1)
                                            {
                                                jogador.reliquias--;
                                            }
                                            PlaySound(jogador.morte);
                                        }
                                    }
                                    else if((int)jogador.corpo.y % QUADRADO != 0)
                                    {//Caso o jogador esteja entre 2 quadrados Y
                                        if(posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][(int)jogador.corpo.x / QUADRADO] == 'X')
                                        {//Testa se houve colisão com espinhos na posição corrigida do jogador
                                            jogador.vida--;//Reinicia o mapa
                                            if(dificuldade == 1)//Se a dificuldade for fácil perde uma vida
                                            {
                                                jogador.vidaTotal--;
                                            }
                                            else//Se fpr difícil o jogador perde todas vidas imediatamente
                                            {
                                                jogador.vidaTotal = 0;
                                            }
                                            //Pontuação e relíquia na fase reiniciadas e pontuação diminuida por uma taxa adicional
                                            jogador.pontuacao -= (jogador.pontuacaoFaseAtual + 100);//Transformar em função PerderVida
                                            if(jogador.reliquiaFaseAtual == 1)
                                            {
                                                jogador.reliquias--;
                                            }
                                            PlaySound(jogador.morte);
                                        }
                                    }
                                    else if((int)jogador.corpo.x % QUADRADO != 0)
                                    {//Caso o jogador esteja entre 2 quadrados em  X
                                        if(posicoesDaTela[((int)jogador.corpo.y - HUD) / QUADRADO][((int)jogador.corpo.x + QUADRADO) / QUADRADO] == 'X' || posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][((int)jogador.corpo.x) / QUADRADO] == 'X' || posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][((int)jogador.corpo.x + QUADRADO) / QUADRADO] == 'X' || posicoesDaTela[((int)jogador.corpo.y - HUD - jogador.velocidadePulo) / QUADRADO][((int)jogador.corpo.x + QUADRADO) / QUADRADO] == 'X' || posicoesDaTela[((int)jogador.corpo.y - HUD - jogador.velocidadePulo) / QUADRADO][((int)jogador.corpo.x) / QUADRADO] == 'X')
                                        {//Testa se houve colisão com espinhos na posição corrigida do jogador
                                            jogador.vida--;//Reinicia o mapa
                                            if(dificuldade == 1)//Se a dificuldade for fácil perde uma vida
                                            {
                                                jogador.vidaTotal--;
                                            }
                                            else//Se fpr difícil o jogador perde todas vidas imediatamente
                                            {
                                                jogador.vidaTotal = 0;
                                            }
                                            //Pontuação e relíquia na fase reiniciadas e pontuação diminuida por uma taxa adicional
                                            jogador.pontuacao -= (jogador.pontuacaoFaseAtual + 100);//Transformar em função PerderVida
                                            if(jogador.reliquiaFaseAtual == 1)
                                            {
                                                jogador.reliquias--;
                                            }
                                            PlaySound(jogador.morte);
                                        }
                                    }
                                    else if(jogador.vida == 1 && posicoesDaTela[((int)jogador.corpo.y - HUD) / QUADRADO][(int)jogador.corpo.x / QUADRADO] == 'X' || posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][(int)jogador.corpo.x / QUADRADO] == 'X' || posicoesDaTela[((int)jogador.corpo.y - HUD - jogador.velocidadePulo) / QUADRADO][(int)jogador.corpo.x / QUADRADO] == 'X')
                                    {//Testa a colisão na posição exata de jogador, tanto quanto se os pés do jogador ou a cabeça estão encostando em um espinho na sua borda
                                        jogador.vida--;//Reinicia o mapa
                                        if(dificuldade == 1)//Se a dificuldade for fácil perde uma vida
                                        {
                                            jogador.vidaTotal--;
                                        }
                                        else//Se fpr difícil o jogador perde todas vidas imediatamente
                                        {
                                            jogador.vidaTotal = 0;
                                        }
                                        //Pontuação e relíquia na fase reiniciadas e pontuação diminuida por uma taxa adicional
                                        jogador.pontuacao -= (jogador.pontuacaoFaseAtual + 100);//Transformar em função PerderVida
                                        if(jogador.reliquiaFaseAtual == 1)
                                        {
                                            jogador.reliquias--;
                                        }
                                        PlaySound(jogador.morte);
                                    }

                                    //Moedas (diversos ifs são necessários para permitir pegar mais de uma moeda por vez
                                    if((int)jogador.corpo.x % QUADRADO != 0 && (int)jogador.corpo.y % QUADRADO != 0)
                                    {//Caso o jogador esteja entre 2 quadrados em  X e Y
                                        if(posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][((int)jogador.corpo.x + QUADRADO) / QUADRADO] == 'C')
                                        {//Testa se houve colisão com uma moeda na posição corrigida do jogador
                                            ColetaColetavel(&jogador.coletaveis, &jogador.pontuacaoFaseAtual, &jogador.pontuacao, coletarColetavel); //Incrementa a contagem de coletáveis e a pontuação do jogador
                                            posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][((int)jogador.corpo.x + QUADRADO) / QUADRADO] = ' '; //Torna o espaço vazio
                                        }
                                    }
                                    if((int)jogador.corpo.y % QUADRADO != 0)
                                    {//Caso o jogador esteja entre 2 quadrados em  Y
                                        if(posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][(int)jogador.corpo.x / QUADRADO] == 'C')
                                        {//Testa se houve colisão com uma moeda na posição corrigida do jogador
                                            ColetaColetavel(&jogador.coletaveis, &jogador.pontuacaoFaseAtual, &jogador.pontuacao, coletarColetavel); //Incrementa a contagem de coletáveis e a pontuação do jogador
                                            posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][(int)jogador.corpo.x / QUADRADO] = ' '; //Torna o espaço vazio
                                        }
                                    }
                                    if((int)jogador.corpo.x % QUADRADO != 0)
                                    {//Caso o jogador esteja entre 2 quadrados em  X
                                        if(posicoesDaTela[((int)jogador.corpo.y - HUD) / QUADRADO][((int)jogador.corpo.x + QUADRADO) / QUADRADO] == 'C')
                                        {//Testa se houve colisão com uma moeda na posição corrigida do jogador
                                            ColetaColetavel(&jogador.coletaveis, &jogador.pontuacaoFaseAtual, &jogador.pontuacao, coletarColetavel); //Incrementa a contagem de coletáveis e a pontuação do jogador
                                            posicoesDaTela[((int)jogador.corpo.y - HUD) / QUADRADO][((int)jogador.corpo.x + QUADRADO) / QUADRADO] = ' '; //Torna o espaço vazio
                                        }
                                    }
                                    if(posicoesDaTela[((int)jogador.corpo.y - HUD) / QUADRADO][(int)jogador.corpo.x / QUADRADO] == 'C')
                                    {//Caso a posição exata do jogador seja uma moeda
                                        ColetaColetavel(&jogador.coletaveis, &jogador.pontuacaoFaseAtual, &jogador.pontuacao, coletarColetavel); //Incrementa a contagem de coletáveis e a pontuação do jogador
                                        posicoesDaTela[((int)jogador.corpo.y - HUD) / QUADRADO][(int)jogador.corpo.x / QUADRADO] = ' '; //Torna o espaço vazio
                                    }

                                    //Portal
                                    if(CheckCollisionRecs(jogador.corpo, (Rectangle){portal.pos.x, portal.pos.y, QUADRADO, QUADRADO}))
                                    {//Como só tem um portal, é mais eficiente comparar a colisão do retângulo do jogador com o do portal
                                        jogador.faseAtual++; //Incrementa a fase atual, indicando que passou de fase
                                        sprintf(mapaAtual, "mapa%d.txt\0", jogador.faseAtual); //Prepara a string do nome do mapa da fase seguinte
                                        jogador.vida--; //Reinica a fase atualizando para o mapa seguinte
                                        jogador.vidaTotal++; //Bônus de 1 de vida a cada mapa completo
                                        jogador.pontuacao += 250; //Bônus de 250 de pontuação por fase completa
                                        EndDrawing();
                                        BeginDrawing();
                                        ClearBackground(RAYWHITE);
                                        //Final do jogo (vitória!)
                                        if(jogador.reliquias == numeroDeMapas)
                                        {//Testa se o número de relíquias é igual ao número de mapas, indicando que o jogador terminou o jogo e coletou todas
                                            PlaySound(fimBom);
                                            DrawTexture(finalBom, 0, 0, WHITE);
                                            EndDrawing();
                                            WaitTime(6); //Exibe durante 6 segundos o final bom
                                            //Fazem o jogador voltar pro menu inicial
                                            comeco = 0;
                                            continuarMenu = 0;
                                            opcao = 1;
                                        }
                                        else if(jogador.faseAtual > numeroDeMapas)
                                        {//Testa se o jogador passou do último mapa previsto, completando o jogo
                                            //Fazem o jogador voltar pro menu inicial
                                            PlaySound(fimRuim);
                                            DrawTexture(finalRuim, 0, 0, WHITE);
                                            EndDrawing();
                                            WaitTime(6); //Exibe durante 6 segundos o final ruim
                                            //Fazem o jogador voltar pro menu inicial
                                            comeco = 0;
                                            continuarMenu = 0;
                                            opcao = 1;
                                        }
                                        else
                                        {//Se o jogo ainda não tiver sido ganho exibe a tela de sucesso e continua
                                            DrawTexture(levelComplete, 0, 0, WHITE);
                                            EndDrawing();
                                            PlaySound(somLevelComplete);
                                            WaitTime(3);
                                        }
                                        EndDrawing();
                                    }

                                    //Reliquias
                                    if(CheckCollisionRecs(jogador.corpo, reliquia.pos))//Testa se a relíquia é coletada
                                    {//Como só tem uma relíquia, é mais eficiente comparar a colisão do retângulo do jogador com o da reliquia
                                        jogador.reliquias++; //Incrementa a contagem de relíquias
                                        jogador.reliquiaFaseAtual = 1; //Indica que a relíquia da fase foi coletada
                                        jogador.pontuacao += 300; //Incrementa a pontuação do jogador
                                        jogador.pontuacaoFaseAtual += 300;
                                        //Zera o retângulo da relíquia para que ela suma
                                        reliquia.pos.x = 0;
                                        reliquia.pos.y = 0;
                                        reliquia.pos.height = 0;
                                        reliquia.pos.width = 0;
                                        PlaySound(coletarReliquia);
                                    }
                                    //Desenho do jogador
                                    if(jogador.sentido != 'b')
                                    {//Desenho do jogador nos sentidos 'e' e 'd', com a primeira linha de sprites do arquivo
                                        DrawTextureRec(jogador.skin, (Rectangle){QUADRADO * (jogador.contador / 5), 0, jogador.corpo.width * jogador.direcao, jogador.corpo.height}, (Vector2){jogador.corpo.x, jogador.corpo.y}, WHITE);
                                    }
                                    else
                                    {//Desenho do jogador no sentido 'b', olhando para baixo, a segunda linha
                                        DrawTextureRec(jogador.skin, (Rectangle){QUADRADO * (jogador.contador / 5), QUADRADO, jogador.corpo.width * jogador.direcao, jogador.corpo.height}, (Vector2){jogador.corpo.x, jogador.corpo.y}, WHITE);
                                    }

                                    if(IsKeyPressed(KEY_R))
                                    {
                                        jogador.vida--;//Reinicia o mapa
                                        jogador.pontuacao -= (jogador.pontuacaoFaseAtual + 100); //Reinicia a pontuação da fase e diminui um valor extra
                                        if(jogador.reliquiaFaseAtual == 1) //Retira a relíquia caso ela tenha sido coletada
                                        {
                                            jogador.reliquias--;
                                        }
                                    }
                                    if(jogador.pontuacao < 0) //Evita que o jogador fique com menos de 0 de pontuação
                                    {
                                        jogador.pontuacao = 0; //A pontuação minima é 0
                                    }
                                    //Desenhos do HUD superior
                                    DrawRectangle(0, 0, LARGURA, HUD, BLUE);
                                    DrawText(TextFormat(("Fase: %d / %d"), jogador.faseAtual, numeroDeMapas), LARGURA / 128, (HUD / 4), 50, BLACK);
                                    DrawText(TextFormat("%s", nomeDoJogo), LARGURA * 17 / 64, (HUD / 4), 40, RED);
                                    if(dificuldade == 1)//Desenha a dificuldade selecionada
                                    {
                                        DrawText(TextFormat(("Dificuldade: Facil"), jogador.faseAtual), LARGURA * 5 / 8, (HUD / 4), 50, BLACK);
                                    }
                                    else
                                    {
                                        DrawText(TextFormat(("Dificuldade: Dificil"), jogador.faseAtual), LARGURA * 5 / 8, (HUD / 4), 50, BLACK);
                                    }
                                    //Desenhos do HUD inferior
                                    DrawRectangle(0, ALTURA - HUD, LARGURA, HUD, BLUE);
                                    DrawTexture(coletavel, LARGURA / 128, ALTURA - (HUD / 2), WHITE);
                                    DrawText(TextFormat(("Coletaveis: %d"), jogador.coletaveis), LARGURA / 32, ALTURA - (HUD / 2), 25, BLACK);
                                    DrawTexture(reliquia.reliquia, LARGURA * 12 / 64, ALTURA - (HUD / 2), WHITE);
                                    DrawText(TextFormat(("Reliquias: %d / %d"), jogador.reliquias, numeroDeMapas), LARGURA * 14 / 64, ALTURA - (HUD / 2), 25, BLACK);
                                    DrawTexture(vida, LARGURA * 53 / 128, ALTURA - (HUD / 2), WHITE);
                                    if(jogador.modoDeus == 1)//Mostra a vida como 999 caso o modo Deus esteja ativado independente da dificuldade, pois a vida no modo é infinita
                                    {
                                        DrawText("Vidas: 999", LARGURA * 28 / 64, ALTURA - (HUD / 2), 25, BLACK);
                                    }
                                    else if(dificuldade == 1)//Mostra a vida caso a dificuldade seja fácil
                                    {
                                        DrawText(TextFormat(("Vidas: %d"), jogador.vidaTotal), LARGURA * 28 / 64, ALTURA - (HUD / 2), 25, BLACK);
                                    }
                                    else//Mostra a vida como 1 caso a dificuldade seja difícil, pois qualquer colisão com inimigos ou espinhos resulta em morte
                                    {
                                        DrawText("Vidas: 1", LARGURA * 28 / 64, ALTURA - (HUD / 2), 25, BLACK);
                                    }
                                    DrawTexture(jogador.objeto, LARGURA * 141 / 256, ALTURA - (HUD / 2) + 5, WHITE);
                                    //Objetos disponíveis
                                    if(jogador.coletaveis < COLETAVEIS_PARA_OBJETO * dificuldade) //0 se o jogador não possuir o mínimo para um objeto
                                    {
                                        DrawText("Objetos Disponiveis: 0", LARGURA * 72 / 128, ALTURA - (HUD / 2), 25, BLACK);
                                    }
                                    else if(jogador.coletaveis < COLETAVEIS_PARA_OBJETO * dificuldade * 2)//Para todos os seguintes, mostra o número disponível pelo número de coletáveis menos o número de objetos arremessados
                                    {
                                        DrawText(TextFormat(("Objetos Disponiveis: %d"), 1 - jogador.objetosArremessados), LARGURA * 72 / 128, ALTURA - (HUD / 2), 25, BLACK);
                                    }
                                    else if(jogador.coletaveis < COLETAVEIS_PARA_OBJETO * dificuldade * 3)
                                    {
                                        DrawText(TextFormat(("Objetos Disponiveis: %d"), 2 - jogador.objetosArremessados), LARGURA * 72 / 128, ALTURA - (HUD / 2), 25, BLACK);
                                    }
                                    else
                                    {
                                        DrawText(TextFormat(("Objetos Disponiveis: %d"), 3 - jogador.objetosArremessados), LARGURA * 72 / 128, ALTURA - (HUD / 2), 25, BLACK);
                                    }
                                    DrawTexture(pontos, LARGURA * 101 / 128, ALTURA - (HUD / 2), WHITE);
                                    DrawText(TextFormat(("Pontos: %d"), jogador.pontuacao), LARGURA * 13 / 16, ALTURA - (HUD / 2), 25, BLACK);
                                    Musica(crumbling_castle,  musica);
                                    ClearBackground(WHITE);
                                    //Game Over!
                                    if(jogador.vidaTotal <= 0 && jogador.modoDeus ==  0)//Se a vida total for 0
                                    {
										//Adicionar uma parte que lê o nome do jogador e armazena sua pontuação
                                        remove(saves.atual); //Remove o arquivo do save atual
                                        //Para a música
                                        StopMusicStream(crumbling_castle);
                                        EndDrawing();
                                        BeginDrawing();
                                        ClearBackground(RAYWHITE);
                                        DrawTexture(gameOver, 0, 0, WHITE);
                                        PlaySound(gameOver1);
                                        PlaySound(gameOver2);
                                        EndDrawing();
                                        WaitTime(3);
										//Fazem o jogador voltar pro menu inicial
                                        comeco = 0;
                                        continuarMenu = 0;
                                        opcao = 1;
                                    }
                                    EndDrawing();
                                    //Menu pause
                                    if(IsKeyPressed(KEY_ESCAPE))
                                    {
                                        EndDrawing();
                                        opcaoPause = 1; //Começa o loop de pause com a opção 1 selecionada
                                        pause = 1; //Mantém o loop de pause
                                        while(pause == 1)
                                        {
                                            BeginDrawing();
                                            Musica(intrasport, musica);
                                            ClearBackground(BLACK);
                                            DrawRectangle(LARGURA / 8, ALTURA * 1 / 10, LARGURA * 6 / 8, ALTURA / 12, BLUE);
                                            DrawRectangle(LARGURA / 8 , ALTURA * 2.5 / 10, LARGURA * 6 / 8, ALTURA / 12, BLUE);
                                            DrawRectangle(LARGURA / 8, ALTURA * 4 / 10, LARGURA * 6 / 8, ALTURA / 12, BLUE);
                                            DrawRectangle(LARGURA / 8, ALTURA * 5.5 / 10, LARGURA * 6 / 8, ALTURA / 12, BLUE);
                                            DrawRectangle(LARGURA / 8, ALTURA * 7 / 10, LARGURA * 6 / 8, ALTURA / 12, BLUE);
                                            DrawRectangle(LARGURA / 8, ALTURA * 8.5 / 10, LARGURA * 6 / 8, ALTURA / 12, BLUE);
                                            switch(opcaoPause)
                                            {//Cada case equivale a uma opção e eles são acessados com enter. Um retângulo mais escuro é desenhado por cima do selecionado
                                                case 1://Opção Continuar Jogo
                                                    DrawRectangle(LARGURA / 8, ALTURA * 1 / 10, LARGURA * 6 / 8, ALTURA / 12, DARKBLUE);
                                                    if(IsKeyPressed(KEY_ENTER))
                                                    {
                                                        PlaySound(selecionar);
                                                        pause = 0; //Quebra o loop de pause
                                                    }
                                                    break;
                                                case 2://Opção Salvar Jogo
                                                    DrawRectangle(LARGURA / 8 , ALTURA * 2.5 / 10, LARGURA * 6 / 8, ALTURA / 12, DARKBLUE);
                                                    if(IsKeyPressed(KEY_ENTER))
                                                    {
                                                        PlaySound(selecionar);
                                                        savesJogo = fopen(saves.atual, "w");//Abre o save com o número desejado e escreve as informações do novo estado de jogo
                                                        for(j = 0; j < LINHAS; j++)
                                                        {
                                                            for(k = 0; k < COLUNAS; k++)
                                                            {
                                                                putc(posicoesDaTela[j][k], savesJogo); //Coloca cada informação de cada posição da matriz do mapa no arquivo (não legível pois não inclui \n
                                                            }
                                                        }
                                                        fprintf(savesJogo, "\n"); //\n para não causar erros na hora da leitura
                                                        fprintf(savesJogo, "%d %d %d %d %d\n", monstrosNoMapa, dificuldade, musica, contadorTextura, jogador.modoDeus); //Salva algumas informações quanto a opções não atreladas a nada e alguns valores que precisam ser lidos antes posteriormente
                                                        for(j = 0; j < MAX_MONSTROS; j++)
                                                        {//Salva todos os atributos de cada um dos monstros
                                                            fprintf(savesJogo, "%d %d %f %f %f %f\n", monstros[j].sentido, monstros[j].contador,monstros[j].corpo.x, monstros[j].corpo.y, monstros[j].corpo.width, monstros[j].corpo.height);
                                                        }
                                                        //Salva todos atributos do jogador
                                                        fprintf(savesJogo, "%f %f %c %d %d %d %d %d %d %d %d %d %d %d\n", jogador.corpo.x, jogador.corpo.y, jogador.sentido, jogador.vidaTotal, jogador.puloAux, jogador.coletaveis, jogador.pontuacao, jogador.pontuacaoFaseAtual, jogador.reliquias, jogador.reliquiaFaseAtual, jogador.faseAtual, jogador.objetosArremessados, jogador.direcao, jogador.contador);
                                                        for(j = 0; j < MAX_OBJETOS; j++)
                                                        {//Salva todos atributos de cada um dos objetos do jogador
                                                            fprintf(savesJogo, "%f %f %f %f %c\n", jogador.objetos[j].x, jogador.objetos[j].y, jogador.objetos[j].width, jogador.objetos[j].height, jogador.sentidoObjetos[j]);
                                                        }
                                                        fprintf(savesJogo, "%f %f %d\n", portal.pos.x, portal.pos.y, portal.contador); //Salva os atributos do portal
                                                        fprintf(savesJogo, "%f %f %f %f\n", reliquia.pos.x, reliquia.pos.y, reliquia.pos.width, reliquia.pos.height); //Salva os atributos da reliquia
                                                        fclose(savesJogo);
                                                    }
                                                    break;
                                                case 3://Opção Config
                                                    DrawRectangle(LARGURA / 8, ALTURA * 4 / 10, LARGURA * 6 / 8, ALTURA / 12, DARKBLUE);
                                                    if(IsKeyPressed(KEY_ENTER))
                                                    {
                                                        if(IsKeyPressed(KEY_ENTER))
                                                        {
                                                            PlaySound(selecionar);
                                                            PlaySound(config);
                                                            Config(intrasport, &musica, controles, introducao, selecionar, &dificuldade, &jogador.vida, &jogador.modoDeus); //Abre o menu de configurações
                                                        }
                                                    }
                                                    break;
                                                case 4://Opção Skins NÃO IMPLEMENTADO A TEMPO INFELIZMENTE, NÃO FAZ NADA
                                                    DrawRectangle(LARGURA / 8, ALTURA * 5.5 / 10, LARGURA * 6 / 8, ALTURA / 12, DARKBLUE);
                                                    if(IsKeyPressed(KEY_ENTER))
                                                    {
                                                        PlaySound(selecionar);
                                                        PlaySound(skins);
                                                    }
                                                    break;
                                                case 5://Opção Menu Principal
                                                    DrawRectangle(LARGURA / 8, ALTURA * 7 / 10, LARGURA * 6 / 8, ALTURA / 12, DARKBLUE);
                                                    if(IsKeyPressed(KEY_ENTER))
                                                    {
                                                        PlaySound(selecionar);
                                                        //Retornam ao menu principal
                                                        pause = 0;
                                                        jogador.vida = 0;
                                                        comeco = 0;
                                                        continuarMenu = 0;
                                                        opcao = 1;
                                                    }
                                                    break;

                                                case 6: //Opção Sair Do Jogo
                                                    DrawRectangle(LARGURA / 8, ALTURA * 8.5 / 10, LARGURA * 6 / 8, ALTURA / 12, DARKBLUE);
                                                    if(IsKeyPressed(KEY_ENTER))
                                                    {
                                                        PlaySound(sair);
                                                        PlaySound(selecionar);
                                                        //Quebram todos laços até sair do jogo
                                                        pause = 0;
                                                        jogador.vida = 0;
                                                        comeco = 0;
                                                        continuarMenu = 0;
                                                        WaitTime(2); //Tempo para mensagem de despedida ser tocado
                                                        fechar = 1; //Faz o loop principal ser quebrado e o jogo se fechar
                                                    }
                                                    break;
                                            }
                                            //Desenhos dos textos das opções
                                            DrawText("Continuar Jogo", LARGURA / 2 - (MeasureText("Continuar Jogo", 60) / 2), ALTURA * 1 / 10, 60, WHITE);
                                            DrawText("Salvar", LARGURA / 2 - (MeasureText("Salvar", 60) / 2), ALTURA * 2.5 / 10, 60, WHITE);
                                            DrawText("Config", LARGURA / 2 - (MeasureText("Config", 60) / 2), ALTURA * 4 / 10, 60, WHITE);
                                            DrawText("Skins", LARGURA / 2 - (MeasureText("Skins", 60) / 2), ALTURA * 5.5 / 10, 60, WHITE);
                                            DrawText("Menu Principal", LARGURA / 2 - (MeasureText("Menu Principal", 60) / 2), ALTURA * 7 / 10, 60, WHITE);
                                            DrawText("Sair Do Jogo", LARGURA / 2 - (MeasureText("Sair Do Jogo", 60) / 2), ALTURA * 8.5 / 10, 60, WHITE);

                                            if(IsKeyPressed(KEY_ESCAPE))
                                            {
                                                pause = 1; //Clicar esc também quebra o laço e sai do menu de pause
                                            }
                                            SelecionadorMenus(&opcaoPause, 6);
                                            EndDrawing();
                                        }
                                    }
                                }
                            }

                        }//Fim da parte da gameplay
                        //Retornar
                        else if(IsKeyPressed(KEY_ENTER) && saves.opcao == 4 && partida == 0)//Caso nenhum save seja selecionado e sim a opção de retorno
                        {
                            continuarMenu = 0; //Quebra o laço da tela de saves e retorna para o menu principal
                            PlaySound(voltar);
                            PlaySound(selecionar);

                        }
                        EndDrawing();
                    }

                }
                break;
			//--------------------------------------------------------------------------------------
            //Configurações
            //--------------------------------------------------------------------------------------
            case 3:
                DrawRectangle(LARGURA / 8, ALTURA * 4.75 / 10, LARGURA * 6 / 8, ALTURA / 12, (Color){80, 3, 6, 255});
                if(IsKeyPressed(KEY_ENTER))
                {
                    PlaySound(selecionar);
                    PlaySound(config);
                    Config(intrasport, &musica, controles, introducao, selecionar, &dificuldade, &jogador.vida, &jogador.modoDeus); //Abre o menu de configurações
                }
                break;
			//--------------------------------------------------------------------------------------
            //Selecionar Skin
            //--------------------------------------------------------------------------------------
            case 4: //NÃO IMPLEMENTADO A TEMPO INFELIZMENTE, NÃO FAZ NADA
                DrawRectangle(LARGURA / 8, ALTURA * 6.25 / 10, LARGURA * 6 / 8, ALTURA / 12, (Color){80, 3, 6, 255});
                if(IsKeyPressed(KEY_ENTER))
                {
                    PlaySound(selecionar);
                    PlaySound(skins);
                }
                break;
			//--------------------------------------------------------------------------------------
            //Placar/Conquistas
            //--------------------------------------------------------------------------------------
            case 5: //NÃO IMPLEMENTADO A TEMPO INFELIZMENTE, NÃO FAZ NADA
                DrawRectangle(LARGURA / 8, ALTURA * 7.75 / 10, LARGURA * 6 / 8, ALTURA / 12, (Color){80, 3, 6, 255});
                if(IsKeyPressed(KEY_ENTER))
                {
                    PlaySound(selecionar);
                    PlaySound(placar);
                }
                break;
			//--------------------------------------------------------------------------------------
            //Sair
            //--------------------------------------------------------------------------------------
            case 6:
                DrawRectangle(LARGURA / 8, ALTURA * 9.25 / 10, LARGURA * 6 / 8, ALTURA / 12, (Color){80, 3, 6, 255});
                if(IsKeyPressed(KEY_ENTER))
                {
                    PlaySound(sair);
                    PlaySound(selecionar);
                    WaitTime(2); //Tempo para mensagem de despedida ser tocado
                    fechar = 1; //Faz o loop principal ser quebrado e o jogo se fechar
                }
                break;
            default:
                break;

        }
        //Textos das opções do menu principal
        DrawText("Novo Jogo", LARGURA / 2 - (MeasureText("Novo Jogo", 60) / 2), ALTURA * 1.75 / 10, 60, WHITE);
        DrawText("Continuar", LARGURA / 2 - (MeasureText("Continuar", 60) / 2), ALTURA * 3.25 / 10, 60, WHITE);
        DrawText("Config", LARGURA / 2 - (MeasureText("Config", 60) / 2), ALTURA * 4.75 / 10, 60, WHITE);
        DrawText("Skins", LARGURA / 2 - (MeasureText("Skins", 60) / 2), ALTURA * 6.25 / 10, 60, WHITE);
        DrawText("Placar", LARGURA / 2 - (MeasureText("Placar", 60) / 2), ALTURA * 7.75 / 10, 60, WHITE);
        DrawText("Sair", LARGURA / 2 - (MeasureText("Sair", 60) / 2), ALTURA * 9.25 / 10, 60, WHITE);
        //Mudar opção marcada
        SelecionadorMenus(&opcao, 6);
        Musica(intrasport, musica);
        EndDrawing();
    }
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
