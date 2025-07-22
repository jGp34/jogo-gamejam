#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define LARGURA 1200 //Largura da tela
#define ALTURA 800 //Altura da tela, na partida s�o 200 de HUD e 600 do jogo no meio
#define NUMERO_SAVES 3 //Numero de saves do jogo
#define DIMENSOES_SAVE 300 //Dimens�es padr�es da textura dos saves
#define HUD 100 //HUD dentro da partida em si, 100 na parte superior e 100 na inferior, eqHUDvale a ALTURA / 8
#define LINHAS 30 //Linhas dos mapas
#define COLUNAS 60 //Colunas dos mapas
#define QUADRADO 20 //EqHUDvale a LARGURA / (COLUNAS -1) e a ALTURA - (2 * HUD) / LINHAS
#define GRAVIDADE 5 //Gravidade padr�o
#define VELOCIDADE_PULO 5 //Velocidade do pulo, mostra quanto o personagem vai subir em um pulo por frame
#define MAX_PULO 140 //Altura m�xima total de um pulo, pode ser ultrapassado com certos valores de pulo mas n�o por mHUDto. MAX_PULO / 20 = quantos blocos de altura valem um pulo maximo
#define VELOCIDADE_MOVIMENTO 4 //Velocidade do movimento horizontal
#define TEMPO_AVISO 600 //Tempo que representa os segundos (divido por 60) entre cada aviso na tela de Controles
#define VIDA_TOTAL_INICIAL 3 //Total de vidas que determina quantas vezes o jogador pode morrer antes de um game over
#define MAX_MONSTROS 15 //Numero maximo de monstros por mapa
#define VELOCIDADE_MONSTROS_PADRAO 1 //Velocidade padr�o que os monstros se movimentam
#define MONSTROS_NOVO_SENTIDO 300 //Tempo em frames (60 fps) para uma nova dire��o do monstro
#define MAX_OBJETOS 3 //Numero maximo de objetos que podem ser arremessados por vez
#define VELOCIDADE_OBJETO 4 //Velocidade do objeto arremessado
#define COLETAVEIS_PARA_OBJETO 10 //Numero de coletaveis necess�rios para um objeto
#define PONTOS_MORTE -100 //Pontua��o ao morrer ou reiniciar
#define PONTOS_MONSTRO 50 //Pontua��o ao matar monstros

typedef struct Saves //Caracteristicas relativas aos saves
{
    int contador; //Contador de cada frame da anima��o
    int lido; //Flag que diz se o save j� foi lido ou n�o
    int opcao;
    char atual[10]; //Nome do arquivo do save selecionado
    Rectangle textura[NUMERO_SAVES]; //Posi��o e tamanho das imagens dos saves
    Sound selec1; //Audio ao abrir save1
    Sound selec2; //Audio ao abrir save1
    Sound selec3; //Audio ao abrir save3
} SAVES;

typedef struct Monstro //Caracteristicas relativas aos monstros
{
    int sentido; //Sentido para qual o Monstro se movimenta, pode ser 1 ou -1
    int contador; //Contagem para que o sentido do monstro muda
    int velocidade; //Dist�ncia que o monstro se move a cada quadro
	Rectangle corpo; //Posi��o do Monstro na tela e seu tamanho
} MONSTRO;

typedef struct Jogador //Caracteristicas relativas ao jogador
{
    Rectangle corpo; //Posi��o de Jogador na tela e seu tamanho
    Rectangle objetos[MAX_OBJETOS]; //Cont�m a posi��o e tamanho de cada objeto do jogador
    int objetosArremessados; //Numero de objetos na tela
    int	vida, vidaTotal; //Determina se houve colis�o com espinhos ou monstros, e quantas vezes o jogador pode colidir com os mesmos antes do "game over"
	int velocidadePulo; //Valor que o personagem vai subir por frame
	int velocidade; //Dist�ncia que o Jogador se move a cada quadro
	int pulo, puloMax, puloAux; //Determinam se o pulo pode ser executado, a altura do pulo em rela��o ao m�ximo e se a gravidade pode voltar a ser aplicada
	int coletaveis; //Contagem de colet�veis coletados na fase atual
	int pontuacao, pontuacaoFaseAtual; //N�mero de pontos conqHUDstados dentre todas fases e na fase atual
	int faseAtual; //Fase em que o jogador est�
	int reliquias, reliquiaFaseAtual; //Contagem de reliquias coletadas no save e na fase atual
	int contador; //Contagem para anima��es
	int direcao; //Pode ser -1 ou 1, serve para as anima��es
	int modoDeus; //Vidas infinitas, projeteis infinitos
	char sentido; //Sentido que o Jogador est� virado e dar� o soco
	char sentidoObjetos[MAX_OBJETOS];
    Texture skin; //Textura de Jogador
    Texture soco; //Textura do soco do Jogador
    Texture objeto; //Textura do objeto arremessavel
    Sound morte; //Efeito sonoro da morte do jogador
    Sound falhaObjeto; //Efeito sonoro de clicar X sem colet�veis o suficiente
    Sound arramessarObjeto; //Efeito sonoro ao arremessar objeto
    Sound socar; //Efeito sonoro de apertar Z
} JOGADOR;

typedef struct Portal //Caracteristicas relativas ao portal
{
    Texture portal; //Textura do Portal
    Vector2 pos; //Posi��o do Portal
    int contador; //Contador para a anima��o do Portal
} PORTAL;

typedef struct Reliquia //Caracteristicas relativas � Reliquia
{
    Texture reliquia; //Textura da Reliquia
    Rectangle pos; //Posi��es e tamanho da Reliquia
} RELIQUIA;

//Fun��o MoveMonstros
//Recebe um Monstro, o array contendo as posi��es na tela e os tamanhos de um quadrado e do HUD e move o monstro para o seu sentido. A cada tempoNovoSentido, gera um novo sentido (que pode ser 1 ou -1 ou em caso de colis�o com paredes ou em risco de cair no ar, inverte o sentido do movimento.
void MoveMonstros(MONSTRO *monstro, char posicoesDaTela[][COLUNAS], int tempoNovoSentido)
{
    if(monstro->contador >= tempoNovoSentido) //A cada quantidade de tempo gera um novo sentido
    {
        do
        {
            monstro->sentido = GetRandomValue(-1, 1);//Gera -1, 0 ou 1, sendo aceitos apenas 1 e -1
        } while(monstro->sentido == 0); //O valor precisa ser necessariamente -1 ou 1, pois 0 o deixaria im�vel
        monstro->contador = 0; //Reincia o contador
    }
    if(posicoesDaTela[((int)monstro->corpo.y - HUD + QUADRADO) / QUADRADO][((int)monstro->corpo.x) / QUADRADO] == ' ' || posicoesDaTela[((int)monstro->corpo.y  - HUD + QUADRADO) / QUADRADO][((int)monstro->corpo.x + QUADRADO + monstro->velocidade) / QUADRADO] == ' ' || posicoesDaTela[((int)monstro->corpo.y - HUD) / QUADRADO][((int)monstro->corpo.x + QUADRADO + monstro->velocidade) / QUADRADO] == '#' || posicoesDaTela[((int)monstro->corpo.y - HUD) / QUADRADO][((int)monstro->corpo.x - monstro->velocidade) / QUADRADO] == '#' || posicoesDaTela[((int)monstro->corpo.y - HUD) / QUADRADO][((int)monstro->corpo.x + QUADRADO + monstro->velocidade) / QUADRADO] == 'B' || posicoesDaTela[((int)monstro->corpo.y - HUD) / QUADRADO][((int)monstro->corpo.x - monstro->velocidade) / QUADRADO] == 'B' || posicoesDaTela[((int)monstro->corpo.y - HUD) / QUADRADO][((int)monstro->corpo.x + QUADRADO + monstro->velocidade) / QUADRADO] == 'X' || posicoesDaTela[((int)monstro->corpo.y - HUD) / QUADRADO][((int)monstro->corpo.x - monstro->velocidade) / QUADRADO] == 'X')
    {//Verifica se  as posi��es do solo a frente e abaixo dos p�s de monstro s�o ar ou se as posi��es diretamente a frente s�o paredes ou blocos destrutive�s
        monstro->sentido *= -1; //Caso sejam, inverte o sentido, evitando a colis�o
        monstro->corpo.x += (monstro->velocidade * monstro->sentido); //Move o monstro no sentido atual
    }
    else //Caso n�o haja colis�o, continua movendo o monstro
    {
        monstro->corpo.x += (monstro->velocidade * monstro->sentido); //Move o monstro no sentido atual
    }
    monstro->contador++; //A cada execu��o o contador vai aumentando para determinar quando 5 segundos se passarem
}

//Fun��o Musica:
//Dados uma musica e uma variavel que representa se o jogo est� mutado, toca m�sica caso o jogo n�o esteja mutado
void Musica(Music som, int musica)
{
    if(musica == 1)//Caso a m�sica n�o esteja mutada
    {
        PlayMusicStream(som);
        UpdateMusicStream(som);
    }
}

//Fun��o Controles:
//Mostra ao jogador a tela dos controles, enquanto toca um aviso para que o jogo continue a cada 10 segundos. Caso uma tecla seja apertada, retorna 1, avisando que a fun��o j� foi exibida uma vez
int Controles(Music som, int musica, Texture textura, Sound introducao, Sound selecionar)
{
    int k = TEMPO_AVISO;//Come�a em 600 para que  o audio seja tocado imeditamente
    while(!(IsKeyPressed(KEY_W) || IsKeyPressed(KEY_A) || IsKeyPressed(KEY_S) || IsKeyPressed(KEY_D) || IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_Z) || IsKeyPressed(KEY_R) || IsKeyPressed(KEY_X) || IsKeyPressed(KEY_ESCAPE)))
    {//Caso qualquer tecla dentre os controles seja pressionada sai do loop
        if(k == TEMPO_AVISO) //Constante que vale 600 e representa que se passaram 60 segundos
        {
            PlaySound(introducao);
            k = 0; //Reinicia o contador
        }
        k++; //Vai aumentando e contando o tempo da repeti��o
        BeginDrawing();
        Musica(som, musica);
        DrawTexture(textura, 0, 0, RAYWHITE); //Desenha a imagem com os controles
        EndDrawing();
    }
    PlaySound(selecionar);
    EndDrawing();
    return 1; //Retorna 1 para evitar que a tela de op��es seja exibida mais de uma vez
}

//Fun��o SelecionadorMenus
//Serve para navegar um menu, alterando o valor de uma vari�vel de op��o baseado na tecla clicada com um limite maximo e come�ando por 1
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

//Fun��o config
//Abre o menu de configura��es, com as op��es voltar, controles, dificuldade, mutar/desmutar, modo Deus e formatar
void Config(Music som, int *musica, Texture textura, Sound introducao, Sound selecionar, int *dificuldade, int *vida, int *modoDeus)
{
    int config = 1, opcaoConfig = 1, k;
    EndDrawing();
    while(config == 1)
    {
        BeginDrawing();
        Musica(som, *musica);
        ClearBackground(BLACK);
		//Desenho dos ret�ngulos das op��es
        DrawRectangle(LARGURA / 8, ALTURA * 1 / 10, LARGURA * 6 / 8, ALTURA / 12, LIME);
        DrawRectangle(LARGURA / 8 , ALTURA * 2.5 / 10, LARGURA * 6 / 8, ALTURA / 12, LIME);
        DrawRectangle(LARGURA / 8, ALTURA * 4 / 10, LARGURA * 6 / 8, ALTURA / 12, LIME);
        DrawRectangle(LARGURA / 8, ALTURA * 5.5 / 10, LARGURA * 6 / 8, ALTURA / 12, LIME);
        DrawRectangle(LARGURA / 8, ALTURA * 7 / 10, LARGURA * 6 / 8, ALTURA / 12, LIME);
        DrawRectangle(LARGURA / 8, ALTURA * 8.5 / 10, LARGURA * 6 / 8, ALTURA / 12, LIME);
        switch(opcaoConfig)
        {//Para cada op��o selecionada, � desenhado um ret�ngulo por cima do primeiro mostrando qual est� selecionada
            case 1: //Op��o voltar
                DrawRectangle(LARGURA / 8, ALTURA * 1 / 10, LARGURA * 6 / 8, ALTURA / 12, DARKGREEN);
                if(IsKeyPressed(KEY_ENTER))
                {
                    PlaySound(selecionar);
                    config = 0; //Quebra o la�o e sai das configura��es
                }
                break;
            case 2://Op��o controles
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
            case 3://Op��o dificuldade
                DrawRectangle(LARGURA / 8, ALTURA * 4 / 10, LARGURA * 6 / 8, ALTURA / 12, DARKGREEN);
                if(IsKeyPressed(KEY_ENTER))
                {
                    if(IsKeyPressed(KEY_ENTER))
                    {
                        PlaySound(selecionar);
                        if(*dificuldade == 1) //Caso a dificuldade seja f�cil
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
            case 4://Op��o mutar/desmutar musica
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
					//Ainda n�o existe placar
                }
                break;
        }
		//Textos das op��es
        DrawText("Voltar", LARGURA / 2 - (MeasureText("Voltar", 60) / 2), ALTURA * 1 / 10, 60, WHITE);
        DrawText("Controles", LARGURA / 2 - (MeasureText("Controles", 60) / 2), ALTURA * 2.5 / 10, 60, WHITE);
        DrawText("Dificuldade", LARGURA / 2 - (MeasureText("Dificuldade", 60) / 2), ALTURA * 4 / 10, 60, WHITE);
        DrawText("Mutar/Desmutar Musica", LARGURA / 2 - (MeasureText("Mutar/Desmutar Musica", 60) / 2), ALTURA * 5.5 / 10, 60, WHITE);
        DrawText("Modo Deus", LARGURA / 2 - (MeasureText("Modo Deus", 60) / 2), ALTURA * 7 / 10, 60, WHITE);
        DrawText("Formatar Saves e Placar", LARGURA / 2 - (MeasureText("Formatar Saves e Placar", 60) / 2), ALTURA * 8.5 / 10, 60, WHITE);
        DrawText("Aviso! Mudar a dificuldade reinicia o mapa!", LARGURA / 2 - (MeasureText("Aviso! Mudar a dificuldade reinicia o mapa!", 30) / 2), ALTURA * 9.5 / 10, 30, RED);
        if(IsKeyPressed(KEY_ESCAPE))
        {
            config = 1; //Quebra o la�o ao apertar esc
        }
        SelecionadorMenus(&opcaoConfig, 6); //Permite a sele��o das op��es com WASD ou as setas
        EndDrawing();
    }
}
//Fun��o ZeraObjetos
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

//Fun��o ZeraMonstros
//Coloca as posi��es de monstro em 0 para o iniciar ou o matar
void ZeraMonstros(Rectangle *monstro)
{
    monstro->height = 0;
    monstro->width = 0;
    monstro->x = 0;
    monstro->y = 0;
}
//Fun��o ColetaColetavel
//Aumenta a contagem de colet�veis e aumenta a pontua��o
void ColetaColetavel(int *coletaveis, int *pontuacaoFaseAtual,  int *pontuacao, Sound coletarColetavel)
{
    int j;
    *coletaveis += 1;
    j = GetRandomValue(25, 50);
    *pontuacao += j;
    *pontuacaoFaseAtual += j;
    PlaySound(coletarColetavel);
}
//Fun��o main
int main(void)
{//Declara��es
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
    //Inicializa��es
    //--------------------------------------------------------------------------------------
    InitWindow(LARGURA, ALTURA, "A Jornada da Freira");//Inicializa janela, com certo tamanho e t�tulo
    InitAudioDevice();//Inicia dispostivi de som
    SetTargetFPS(60);// Ajusta a execu��o do jogo para 60 frames por segundo
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
    Sound introducao = LoadSound("./sfx/introdu��o.mp3");
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
	//Inicia��o de alguns atributos com constantes
    jogador.velocidadePulo = VELOCIDADE_PULO;
    jogador.corpo.width = QUADRADO;
    jogador.corpo.height = QUADRADO;
    jogador.velocidade = VELOCIDADE_MOVIMENTO;
    jogador.modoDeus = 0;
    SetRandomSeed(time(NULL));
    //Contagem de mapas
    for(j = 1; k == 0; j++)//Conta quantos mapas existem at� o mapa segHUDnte nao ser aberto
    {
        sprintf(mapaAtual, "mapa%d.txt\0", j);//Vai montando strings com o nome do mapa com cada valor de J
        if(!(mapa = fopen(mapaAtual, "r")))//Caso n�o seja poss�vel ler o mapa
        {
            k = 1;//Determina que o �ltimo mapa foi contador
        }
        else
        {
            numeroDeMapas++;//Mais um mapa � contador
            fclose(mapa);//Mapa fechado
        }
    }
    //Inicia a textura dos saves para posterior exibi��o
    for(j = 0; j < NUMERO_SAVES; j++)
    {
        saves.textura[j].x = 0;
        saves.textura[j].y = 0;
        saves.textura[j].width = 300;
        saves.textura[j].height = 300;
    }
    //--------------------------------------------------------------------------------------
    //La�o Do Jogo
    //--------------------------------------------------------------------------------------
    while (fechar == 0) //O jogo s� ir� se fechar quando a vari�vel fechar for diferente de 0. Isso s� occore clicando em sair no menu principal.
    {
        //--------------------------------------------------------------------------------------
        //Menu Principal
        //--------------------------------------------------------------------------------------
        if(iniciou == 0) //Tela de controles, exibida uma �nica vez ao iniciar o jogo
        {
            iniciou = Controles(intrasport, musica, controles, introducao, selecionar);
        }
        BeginDrawing();
        ClearBackground(BLACK);
		//Titulo
        DrawRectangle(LARGURA / 2 - (MeasureText(nomeDoJogo, 80) / 2), ALTURA * 0.25 / 10, MeasureText(nomeDoJogo, 80), ALTURA / 10, BLUE);
        DrawText(nomeDoJogo, (LARGURA / 2) - (MeasureText(nomeDoJogo, 80) / 2), ALTURA * 0.25 / 10 , 80, WHITE);
        //Retangulos das op��es
        DrawRectangle(LARGURA / 8, ALTURA * 1.75 / 10, LARGURA * 6 / 8, ALTURA / 12, RED);
        DrawRectangle(LARGURA / 8 , ALTURA * 3.25 / 10, LARGURA * 6 / 8, ALTURA / 12, RED);
        DrawRectangle(LARGURA / 8, ALTURA * 4.75 / 10, LARGURA * 6 / 8, ALTURA / 12, RED);
        DrawRectangle(LARGURA / 8, ALTURA * 6.25 / 10, LARGURA * 6 / 8, ALTURA / 12, RED);
        DrawRectangle(LARGURA / 8, ALTURA * 7.75 / 10, LARGURA * 6 / 8, ALTURA / 12, RED);
        DrawRectangle(LARGURA / 8, ALTURA * 9.25 / 10, LARGURA * 6 / 8, ALTURA / 12, RED);

        opcaoContinuar = 0; //Deixa a op��o como 0 para caso a op��o selecionada seja Novo Jogo
        //--------------------------------------------------------------------------------------
        //Opc��es do Menu Principal
        //--------------------------------------------------------------------------------------
        switch(opcao) //Retangulos s�o desenhados por cima dos originais pra mostrar a op��o selecionada
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
                    opcaoContinuar = 1; //Faz com quem os arquivos sejam tirados do save e n�o do mapa
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
                    if(opcaoContinuar == 0) //Garante que o audio n�o sera executado para a segunda op��o
                    {
                        PlaySound(selecionar);
                        PlaySound(novoJogo);
                    }
                    continuarMenu = 1; //Mantem a sele��o de save em loop
                    saves.opcao = 1; //Inicia a op��o do save na primeira op��o
                    saves.contador = 0; //Conta o tempo entre cada mudan�a da anima��o dos saves
                    partida = 0; //Quando 1, continua a inicializa��o da partida e evita que o save selecionado seja mudado durante o carregamento
                    comeco = 0; //Determina o fim da anima��o e o come�o do jogo em si
                    saves.lido = 0; //Se o save j� foi lido ou criado vira 1
					//--------------------------------------------------------------------------------------
                    //Sele��o de Save
                    //--------------------------------------------------------------------------------------
                    while(continuarMenu == 1)
                    {

                        Musica(intrasport, musica);
                        BeginDrawing();
                        ClearBackground(BLACK);
						//Textos da tela de sele��o de save
                        DrawText("Escolha um save para jogar!", (LARGURA / 2) - (MeasureText("Escolha um save para jogar!", 60) / 2), ALTURA * 0.25 / 10 , 60, RAYWHITE);
                        DrawText("Save 1", (LARGURA / 16) + (DIMENSOES_SAVE / 2) - (MeasureText("Save 1", 30) / 2), ALTURA * 2.6 / 10 , 30, RED);
                        DrawText("Save 2", (LARGURA * 3 / 8) + (DIMENSOES_SAVE / 2) - (MeasureText("Save 2", 30) / 2), ALTURA * 2.6 / 10 , 30, RED);
                        DrawText("Save 3", (LARGURA * 11 / 16) + (DIMENSOES_SAVE / 2) - (MeasureText("Save 3", 30) / 2), ALTURA * 2.6 / 10 , 30, RED);
                        //Desenha as texturas dos ret�ngulos dos saves e o ret�ngulo para retornar
                        DrawTextureRec(save, saves.textura[0], (Vector2){LARGURA / 16, ALTURA * 3.2 / 10}, WHITE);
                        DrawTextureRec(save, saves.textura[1], (Vector2){LARGURA * 3 / 8, ALTURA * 3.2 / 10}, WHITE);
                        DrawTextureRec(save, saves.textura[2], (Vector2){LARGURA * 11 / 16, ALTURA * 3.2 / 10}, WHITE);
                        DrawRectangle(0, ALTURA  * 15 / 16, LARGURA * 7 / 16, ALTURA  * 15 / 16, GRAY);
						//Seletor de op��o na tela dos saves
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
                            saves.opcao = 4; //Op��o 4 � voltar e est� na parte inferior da tela
                        }
                        else if((IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) && saves.opcao == 4 && partida == 0)
                        {
                            saves.opcao = 2;
                        }
						//Desenha setas apontando para a op��o que se pretende abrir ou um ret�ngulo para a op��o voltar
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

                            case 4://Ret�ngulo de cor diferente
                                DrawRectangle(0, ALTURA  * 15 / 16, LARGURA * 7 / 16, ALTURA  * 15 / 16, RAYWHITE);
                                break;

                        }
                        DrawText("Selecione para voltar", 10, ALTURA  * 15 / 16, 40, GOLD); //Texto da op��o de voltar
						//Op��o de save selecionada
                        if((IsKeyPressed(KEY_ENTER) || partida == 1) && saves.opcao != 4) //Executa se a op��o n�o for a 3 (que � o retorno) e continua em execu��o caso tenha sido selecionada pela primeira vez com enter
                        {
                            partida = 1;//Para condi��o continue e a anima��o termine
                            PlaySound(selecionar);
							if(saves.lido == 0) //Executa se os saves ainda n�o foram lidos
                            {
                                        sprintf(saves.atual, "save%d.txt\0", saves.opcao);//Determina o nome do save a ser aberto ou excluido com base na op��o de save
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
                                        if(opcaoContinuar == 1) //Caso a op��o seja continuar jogo
                                        {
                                            if(!(savesJogo = fopen(saves.atual, "r")))//Caso o save n�o tenha sido aberto
                                            {
                                                remove(saves.atual); //Remove qualquer arquivo de save que existir com o numero selecionado
												//Inicia��o de um save novo, atributos do jogador iniciados
                                                jogador.vidaTotal = VIDA_TOTAL_INICIAL;
                                                jogador.pontuacao = 0;
                                                jogador.reliquias = 0;
                                                jogador.faseAtual = 1;
                                                jogador.pontuacao = 0;
                                                jogador.vida = 0; //Come�a em 0 pra garantir a leitura adequada do mapa posteriormente
                                            }
                                            else //Caso ele tenha sido aberto
                                            {
                                                for(j = 0; j < LINHAS; j++)
                                                {
                                                    for(k = 0; k < COLUNAS; k++)
                                                    {
                                                        posicoesDaTela[j][k] = fgetc(savesJogo); //L� cada posi��o do mapa do arquivo
                                                    }
                                                }
                                                fscanf(savesJogo, "%d %d %d %d %d", &monstrosNoMapa, &dificuldade, &musica, &contadorTextura, &jogador.modoDeus); //L� algumas outras informa��es sobre configura��es e sobre o mapa
                                                for(j = 0; j < MAX_MONSTROS; j++)
                                                {//L� todos os detalhes de cada monstro
                                                    fscanf(savesJogo, "%d %d %f %f %f %f\n", &monstros[j].sentido, &monstros[j].contador, &monstros[j].corpo.x, &monstros[j].corpo.y, &monstros[j].corpo.width, &monstros[j].corpo.height);
                                                    monstros[j].velocidade = VELOCIDADE_MONSTROS_PADRAO * dificuldade; //A velocidade � constante mas depende da dificuldade
                                                }
												//L� todas informa��es relacionadas ao jogador
                                                fscanf(savesJogo, "%f %f %c %d %d %d %d %d %d %d %d %d %d %d\n", &jogador.corpo.x, &jogador.corpo.y, &jogador.sentido, &jogador.vidaTotal, &jogador.puloAux, &jogador.coletaveis, &jogador.pontuacao, &jogador.pontuacaoFaseAtual, &jogador.reliquias, &jogador.reliquiaFaseAtual, &jogador.faseAtual, &jogador.objetosArremessados, &jogador.direcao, &jogador.contador);
                                                for(j = 0; j < MAX_OBJETOS; j++)
                                                {//L� informa��es relacionadas aos objetos do jogador
                                                    fscanf(savesJogo, "%f %f %f %f %c\n", &jogador.objetos[j].x, &jogador.objetos[j].y, &jogador.objetos[j].width, &jogador.objetos[j].height, &jogador.sentidoObjetos[j]);
                                                }
                                                fscanf(savesJogo, "%f %f %d\n", &portal.pos.x, &portal.pos.y, &portal.contador); //L� informa��es do portal
                                                fscanf(savesJogo, "%f %f %f %f\n", &reliquia.pos.x, &reliquia.pos.y, &reliquia.pos.width, &reliquia.pos.height); //L� informa��es da reliquia
                                                jogador.vida = 1; //Permite que o loop de gameplay ocorra e evita que o mapa seja lido
                                                lerMapa = 1; //Tamb�m ajuda a evitar que o mapa seja lido
                                                sprintf(mapaFundo, "./images/fundo%d.png", jogador.faseAtual);//Determina qual imagem de fundo deve ser carregada
                                                UnloadTexture(fundo);//Limpa qualquer fundo j� carregado
                                                fundo = LoadTexture(mapaFundo); //Carrega o fundo desejado
                                                if(fundo.id <= 0) //Caso nenhum fundo tenha sido carregado
                                                {
                                                    fundo = LoadTexture("./images/fundo1.png"); //Carrega o fundo1 para n�o ficar sem fundo
                                                }
                                                fclose(savesJogo);
                                            }
                                        }
                                        else
                                        {
                                            remove(saves.atual); //Remove qualquer arquivo de save que existir com o numero selecionado
                                            //Inicia��o de um save novo, atributos do jogador iniciados
                                            jogador.vidaTotal = VIDA_TOTAL_INICIAL;
                                            jogador.pontuacao = 0;
                                            jogador.reliquias = 0;
                                            jogador.faseAtual = 1;
                                            jogador.pontuacao = 0;
                                            jogador.vida = 0; //Come�a em 0 pra garantir a leitura adequada do mapa posteriormente
                                        }
                                        sprintf(mapaAtual, "mapa%d.txt\0", jogador.faseAtual); //Define o mapa atual como o informado na faseAtual
                                        saves.lido = 1; //Save j� foi lido

                            }
                            saves.contador++; //Atualiza o contador pra anima��o da textura
                            if(saves.contador == 10) //Atualiza a textura a cada dez frames
                            {
                                saves.contador = 0; //Reinicia a contagem
                                saves.textura[saves.opcao - 1].x += 300; //Atualiza a textura do respectivo save
                                comeco++; //Conta quantos quadros da anima��o j� foram mostrados
                            }
							//--------------------------------------------------------------------------------------
                            //Loop da gameplay
                            //--------------------------------------------------------------------------------------
                            while(comeco == 6) //Executa quando o �ltimo quadro da anima��o for mostrado
                            {
                                StopMusicStream(intrasport);
                                if(jogador.vida == 0) //S� executa enquanto a fase n�o for reiniciada(ou uma nova iniciada) e o save n�o tiver sido lido
                                {
                                    lerMapa = 0; //Garante que o mapa seja lido apenas uma vez
                                }
                                jogador.vida = 1; //Define o n�mero de ataques que o jogador pode tomar antes de reiniciar a fase como 1
                                while(jogador.vida > 0)
                                {
                                    if(lerMapa == 0) //Testa se o mapa ainda n�o foi lido
                                    {
                                        if(!(mapa = fopen(mapaAtual, "r"))) //Abre o mapa, e caso falhe
                                        {
                                            PlaySound(falhaAbrirMapa);
                                            comeco = 0; //Sai do loop de gameplay
                                            continuarMenu = 0; //Sai do menu de saves
                                        }
                                        else //Caso funcione
                                        {
                                            monstrosNoMapa = 0;//Contagem de monstros come�a como 0
                                            for(j = 0; j < LINHAS; j++)
                                            {
                                                for(k = 0; k < COLUNAS + 1; k++) //Passa pelos \n no fim de cada linha
                                                {
                                                    if(k < COLUNAS) //Caso o valor seja qualquer um menos \n
                                                    {
                                                        posicoesDaTela[j][k] = fgetc(mapa); //Preenche a matriz de posi��es na tela com os caracteres de cada posi��o do mapa
		                                                if(posicoesDaTela[j][k] == 'Z') //Caso a posi��o seja um Monstro
														{
															//Tamanho e coordenadas do monstro iniciadas
															monstros[monstrosNoMapa].corpo.width = QUADRADO;
															monstros[monstrosNoMapa].corpo.height = QUADRADO;
															monstros[monstrosNoMapa].corpo.x = k * QUADRADO;//Posi��o x iniciada
															monstros[monstrosNoMapa].corpo.y =  HUD + (j * QUADRADO); //Posi��o y iniciada, HUD � somado devido ao HUD na parte de cima da tela
															monstros[monstrosNoMapa].velocidade = VELOCIDADE_MONSTROS_PADRAO * dificuldade; //Inicia a velocidade dos monstros
															monstros[monstrosNoMapa].contador = MONSTROS_NOVO_SENTIDO; //O contador para uma nova dire��o do monstro come�a no com o valor que define um novo sentido para que o sentido inicial seja aleat�rio
															monstrosNoMapa++; //Um monstro extra � adicionado a contagem
															posicoesDaTela[j][k] = ' '; //A posi��o pois n�o � mais necess�ria
														}
														else if(posicoesDaTela[j][k] == 'J')//Caso a posi��o seja o Jogador
														{
															jogador.corpo.x = k * QUADRADO; //Posi��o x iniciada
															jogador.corpo.y = HUD + (j * QUADRADO); //Posi��o y iniciada, HUD � somado devido ao HUD na parte de cima da tela
															posicoesDaTela[j][k] = ' '; //A posi��o pois n�o � mais necess�ria
														}
														else if(posicoesDaTela[j][k] == 'P')//Caso a posi��o seja o portal
														{
															portal.pos.x = k * QUADRADO; //Posi��o x iniciada
															portal.pos.y = HUD + (j * QUADRADO); //Posi��o y iniciada, HUD � somado devido ao HUD na parte de cima da tela
															posicoesDaTela[j][k] = ' '; //A posi��o pois n�o � mais necess�ria
														}
														else if(posicoesDaTela[j][k] == 'H')//Caso a posi��o seja a reliquia
														{
															reliquia.pos.x = k * QUADRADO; //Posi��o x iniciada
															reliquia.pos.y = HUD + (j * QUADRADO); //Posi��o y iniciada, HUD � somado devido ao HUD na parte de cima da tela
															reliquia.pos.height = QUADRADO;
															reliquia.pos.width = QUADRADO;
															posicoesDaTela[j][k] = ' '; //A posi��o pois n�o � mais necess�ria
														}
                                                    }
                                                    else //Isso significa que a linha acabou e o pr�ximo valor � um \n, que deve ser ignorado
                                                    {
                                                        fgetc(mapa); //N�o armazena \n
                                                    }
                                                }
                                            }
                                            for(j = monstrosNoMapa; j  < MAX_MONSTROS; j++)
                                            {//Zera todos as vari�veis dos outros monstros inexistentes para que n�o atrapalhem
										        ZeraMonstros(&monstros[j].corpo);
                                                monstros[j].sentido = 0;
                                                monstros[j].contador = 0;
                                            }
                                            lerMapa = 1; //Evita que o  mapa seja lido mais de uma vez
                                            fclose(mapa); //Fecha arquvio ap�s leitura
                                            jogador.puloAux = 0;  //Atua na gravidade, para que seja aplicada apenas enquanto n�o est� pulando
                                            jogador.pulo = 0; //Evita que o jogador pule antes de atingir solo
                                            jogador.puloMax = 0; //Mostra que a altura maxima de pulo n�o foi alcan�ada ainda
                                            jogador.sentido = 'd'; //O jogador come�a virado para a direita
                                            jogador.coletaveis = 0; //Inicia o total de colet�veis em 0
                                            jogador.pontuacaoFaseAtual = 0; //A pontua��o adquiadqHUDridarida na fase atual come�a em 0 a cada morte ou novo come�o
                                            jogador.reliquiaFaseAtual = 0; //Determina se a reliquia da fase atual foi coletada
                                            jogador.objetosArremessados = MAX_OBJETOS; //Contagem de objetos come�a em MAX_OBJETOS pois a fun��o posterior o diminui por este valor
                                            portal.contador = 0; //Come�a a anima��o do portal em 0
                                            jogador.contador = 0; //Contador da anima��o do jogador come�a em 0
                                            jogador.direcao =  -1;//Dire��o da textura do jogador come�a para direita
                                            contadorTextura = 0; //Inicia a textura no 0
											sprintf(mapaFundo, "./images/fundo%d.png", jogador.faseAtual);//Determina qual imagem de fundo deve ser carregada
											UnloadTexture(fundo);//Limpa qualquer fundo j� carregado
											fundo = LoadTexture(mapaFundo); //Carrega o fundo desejado
											if(fundo.id <= 0) //Caso nenhum fundo tenha sido carregado
											{
												fundo = LoadTexture("./images/fundo1.png"); //Carrega o fundo1 para n�o ficar sem fundo
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
                                    //Desenho de cada espa�o na tela
                                    for(j = 0; j < LINHAS; j++)
                                    {
                                        for(k = 0; k < COLUNAS; k++)
                                        {
                                            if(posicoesDaTela[j][k] == '#') //Desenha as parades
                                            {
                                                DrawTexture(blocoNormal, k * QUADRADO, HUD + j * QUADRADO, WHITE);
                                            }
                                            else if(posicoesDaTela[j][k] == 'B') //Desenha os blocos quebr�veis
                                            {
                                                DrawTexture(blocoQuebravel, k * QUADRADO, HUD + j * QUADRADO, WHITE);
                                            }
                                            else if(posicoesDaTela[j][k] == 'C') //Desenha os colet�veis
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
                                        if(CheckCollisionRecs(jogador.corpo, monstros[j].corpo))//Colis�o com os monstros
                                        {
                                            jogador.vida--; //Reinicia a fase
                                            if(dificuldade == 1)
                                            {
                                                jogador.vidaTotal--; //Dificuldade facil diminui uma vida
                                            }
                                            else
                                            {
                                                jogador.vidaTotal = 0; //Dificuldade dificil game over instant�neo
                                            }
                                            jogador.pontuacao += (PONTOS_MORTE - jogador.pontuacaoFaseAtual); //Pontua��o � voltada ao valor do inicio da fase e tirado uma constante
                                            if(jogador.reliquiaFaseAtual == 1) //Caso a reliquia da fase tenha sido pegada, retira ela
                                            {
                                                jogador.reliquias--;
                                            }
                                            PlaySound(jogador.morte);
                                        }
                                        for(k = 0; k < MAX_OBJETOS; k++)
                                        {
                                            if(CheckCollisionRecs(jogador.objetos[k], monstros[j].corpo)) //Caso o monstro colida com um objeto arremessado pelo jogador
                                            {//Zera tanto o monstro como o objeto e acrescenta a pontua��o do jogador
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
                                    {//Testa se o jogador.corpo.y est� entre 2 quadrados
                                        if((IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) && (posicoesDaTela[((int)jogador.corpo.y - HUD) / QUADRADO][((int)jogador.corpo.x + QUADRADO) / QUADRADO] != '#' && posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][((int)jogador.corpo.x + QUADRADO) / QUADRADO] != '#') && (posicoesDaTela[((int)jogador.corpo.y - HUD) / QUADRADO][((int)jogador.corpo.x + QUADRADO) / QUADRADO] != 'B' && posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][((int)jogador.corpo.x + QUADRADO) / QUADRADO] != 'B'))
                                        {//Verifica se a posi��o a direita dos 2 quadrados que o jogador est� entre � um obst�culo
                                            jogador.corpo.x += jogador.velocidade; //Move para a direita
                                        }
                                        else if((IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) && (posicoesDaTela[((int)jogador.corpo.y - HUD) / QUADRADO][((int)jogador.corpo.x - jogador.velocidade) / QUADRADO] != '#' && posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][((int)jogador.corpo.x - jogador.velocidade) / QUADRADO] != '#') && (posicoesDaTela[((int)jogador.corpo.y - HUD) / QUADRADO][((int)jogador.corpo.x - jogador.velocidade) / QUADRADO] != 'B' && posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][((int)jogador.corpo.x - jogador.velocidade) / QUADRADO] != 'B'))
                                        {//Verifica se a posi��o a esquerda dos 2 quadrados que o jogador est� entre � um obst�culo
                                            jogador.corpo.x -= jogador.velocidade; //Move para a esquerda
                                        }
                                    }
                                    else//Como � divis�vel por 20, s� � necess�rio testar um quadrado
                                    {
                                        if((IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) && (posicoesDaTela[((int)jogador.corpo.y - HUD) / QUADRADO][((int)jogador.corpo.x + QUADRADO) / QUADRADO] != '#') && (posicoesDaTela[((int)jogador.corpo.y - HUD) / QUADRADO][((int)jogador.corpo.x + QUADRADO) / QUADRADO] != 'B'))
                                        {//Verifica se a posi��o a direita do quadrado que o jogador est� � um obst�culo
                                            jogador.corpo.x += jogador.velocidade; //Move para a direita
                                        }
                                        else if((IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) && (posicoesDaTela[((int)jogador.corpo.y - HUD) / QUADRADO][((int)jogador.corpo.x - jogador.velocidade) / QUADRADO] != '#') && (posicoesDaTela[((int)jogador.corpo.y - HUD) / QUADRADO][((int)jogador.corpo.x - jogador.velocidade) / QUADRADO] != 'B'))
                                        {//Verifica se a posi��o a esquerda do quadrado que o jogador est� � um obst�culo
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
                                    {//puloAux diz se a gravidade pode continuar e testa se jogador.corpo.x est� entre 2 quadrados
                                        if(posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][(int)jogador.corpo.x / QUADRADO] != '#' && posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][((int)jogador.corpo.x + QUADRADO) / QUADRADO] != '#' && posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][(int)jogador.corpo.x / QUADRADO] != 'B' && posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][((int)jogador.corpo.x + QUADRADO) / QUADRADO] != 'B')
                                        {//Verifica se a posi��o abaixo dos 2 quadrados que o jogador est� entre � um obst�culo
                                            jogador.corpo.y += GRAVIDADE; //Move para baixo
                                            jogador.pulo = 0; //N�o permite continuar o pulo
                                        }
                                        else
                                        {
                                            jogador.pulo = 1;//Permite continuar o pulo
                                            jogador.puloAux = 0; //Permite a gravidade continuar
                                        }

                                    }
                                    else if(jogador.puloAux == 0 && posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][(int)jogador.corpo.x / QUADRADO] != '#' && posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][(int)jogador.corpo.x / QUADRADO] != 'B') //Como � divis�vel por 20, s� � necess�rio testar um quadrado
                                    {//Verifica se a posi��o abaixo do quadrado que o jogador est� � um obst�culo
                                        jogador.corpo.y += GRAVIDADE;//Move para baixo
                                        jogador.pulo = 0; //N�o permite continuar o pulo
                                    }
                                    else
                                    {
                                            jogador.pulo = 1; //Permite continuar o pulo
                                            jogador.puloAux = 0; //Permite a gravidade continuar
                                    }

                                    //Pulo
                                    if((IsKeyReleased(KEY_SPACE) || IsKeyReleased(KEY_W) || IsKeyReleased(KEY_UP)) ||  jogador.puloMax >= MAX_PULO)
                                    {//Verifica se a tecla de pulo � largada ou a altura do pulo atingiu seu m�ximo, finalizando o pulo
                                        jogador.pulo = 0; //N�o permite continuar o pulo
                                        jogador.puloMax = 0;  //Reinicia a altura m�xima de pulo
                                        jogador.puloAux = 0; //Permite a gravidade continuar
                                    }
                                    else if((int)jogador.corpo.x % QUADRADO != 0)//Caso jogador.corpo.x esteja entre 2 quadrados, � necess�rio uma verifica��o com 2 testes
                                    {
                                        if(posicoesDaTela[((int)jogador.corpo.y - HUD - jogador.velocidadePulo) / QUADRADO][(int)jogador.corpo.x / QUADRADO] == '#' || posicoesDaTela[((int)jogador.corpo.y - HUD - jogador.velocidadePulo) / QUADRADO][((int)jogador.corpo.x + QUADRADO) / QUADRADO] == '#' || posicoesDaTela[((int)jogador.corpo.y - HUD - jogador.velocidadePulo) / QUADRADO][(int)jogador.corpo.x / QUADRADO] == 'B' || posicoesDaTela[((int)jogador.corpo.y - HUD - jogador.velocidadePulo) / QUADRADO][((int)jogador.corpo.x + QUADRADO) / QUADRADO] == 'B')
                                        {//Verifica se a posi��o acima dos 2 quadrados que o jogador est� entre � um obst�culo, caso sim encerra o pulo
                                            jogador.pulo = 0; //N�o permite continuar o pulo assim que a tecla � largada
                                            jogador.puloMax = 0;  //Reinicia a altura m�xima de pulo
                                            jogador.puloAux = 0; //Permite a gravidade continuar
                                        }
                                        else if((IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) && jogador.pulo == 1 && jogador.puloMax <= MAX_PULO)
                                        {//Pula apenas caso o jogador esteja no solo e a altura do pulo esteja menor que o m�ximo
                                            jogador.corpo.y -= jogador.velocidadePulo; //Faz o jogador ir para cima baseado no valor do pulo por vez, permitindo tamanhos diferentes de pulo
                                            jogador.puloMax += jogador.velocidadePulo; //Aumenta o quanto foi pulado at� ent�o, desscontada a gravidade
                                            jogador.puloAux = 1; //N�o permite a gravidade continuar
                                        }
                                    }
                                    else if(posicoesDaTela[((int)jogador.corpo.y - HUD - jogador.velocidadePulo) / QUADRADO][(int)jogador.corpo.x / QUADRADO] == '#' || posicoesDaTela[((int)jogador.corpo.y - HUD - jogador.velocidadePulo) / QUADRADO][(int)jogador.corpo.x / QUADRADO] == 'B')
                                    {//Caso jogador.corpo.x seja divis�vel por QUADRADO, faz apenas um teste, e caso a posi��o acima do jogador tenha um obst�culo, encerra o pulo
                                            jogador.pulo = 0; //N�o permite continuar o pulo assim que a tecla � largada
                                            jogador.puloMax = 0;  //Reinicia a altura m�xima de pulo
                                            jogador.puloAux = 0; //Permite a gravidade continuar
                                    }
                                    else if((IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) && jogador.pulo == 1 && jogador.puloMax <= MAX_PULO)
                                    {//Pula apenas caso o jogador esteja no solo e a altura do pulo esteja menor que o m�ximo
                                        jogador.corpo.y -= jogador.velocidadePulo; //Faz o jogador ir para cima baseado no valor do pulo por vez, permitindo tamanhos diferentes de pulo
                                        jogador.puloMax += jogador.velocidadePulo; //Aumenta o quanto foi pulado at� ent�o, desscontada a gravidade
                                        jogador.puloAux = 1; //N�o permite a gravidade continuar
                                    }
                                    //Soco
                                    if(IsKeyPressed(KEY_Z))
                                    {
                                        PlaySound(jogador.socar);
                                        switch(jogador.sentido) //O soco vai ser voltado para o sentido que o jogador est�
                                        {//Todos os testes servem para garantir que, independente da posi��o do Jogador, sempre seja contado a �rea precisa do soco, evitando que blocos sejam "atingidos" mas n�o quebrados
                                            case 'd'://Caso direita
                                                DrawTextureRec(jogador.soco, (Rectangle){0, 0, -QUADRADO, QUADRADO},(Vector2){jogador.corpo.x + QUADRADO, jogador.corpo.y}, WHITE); //Desenho do soco a direita de Jogador
                                                if(posicoesDaTela[((int)jogador.corpo.y - HUD) / QUADRADO][((int)jogador.corpo.x + QUADRADO) / QUADRADO] == 'B')
                                                {//Teste se a posi��o a direita do jogador � um bloco quebr�vel
                                                    posicoesDaTela[((int)jogador.corpo.y - HUD) / QUADRADO][((int)jogador.corpo.x + QUADRADO) / QUADRADO] = 'C'; //Transforma bloco em colet�vel
                                                    PlaySound(quebrarBlocos);
                                                }
                                                else if((int)jogador.corpo.x % QUADRADO != 0)
                                                {//Caso o jogador esteja entre 2 posi��es X, testa se a 2 posi��es a direita � um bloco quebr�vel
                                                    if(posicoesDaTela[((int)jogador.corpo.y - HUD) / QUADRADO][((int)jogador.corpo.x + (2 * QUADRADO)) / QUADRADO] == 'B')
                                                    {
                                                        posicoesDaTela[((int)jogador.corpo.y - HUD) / QUADRADO][((int)jogador.corpo.x + (2 * QUADRADO)) / QUADRADO] = 'C'; //Transforma bloco em colet�vel
                                                        PlaySound(quebrarBlocos);
                                                    }
                                                }
                                                if((int)jogador.corpo.y % QUADRADO != 0)
                                                {//Caso o jogador esteja entre 2 posi��es Y
                                                    if(posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][((int)jogador.corpo.x + QUADRADO) / QUADRADO] == 'B')
                                                    {//Teste se a posi��o a direita da outra posi��o Y que Jogador est� entre � um bloco quebr�vel
                                                        posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][((int)jogador.corpo.x + QUADRADO) / QUADRADO] = 'C'; //Transforma bloco em colet�vel
                                                        PlaySound(quebrarBlocos);
                                                    }
                                                    else if((int)jogador.corpo.x % QUADRADO != 0)
                                                    {//Caso ainda Jogador esteja entre 2 posi��es X e 2 posi��es Y, testa a se 2 posi��es a direita da outra posi��o Y � quebr�vel
                                                        if(posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][((int)jogador.corpo.x + (2 * QUADRADO)) / QUADRADO] == 'B')
                                                        {
                                                            posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][((int)jogador.corpo.x + (2 * QUADRADO)) / QUADRADO] = 'C'; //Transforma bloco em colet�vel
                                                            PlaySound(quebrarBlocos);
                                                        }
                                                    }
                                                }
                                                for(j = 0; j < monstrosNoMapa; j++)//Verifica a colis�o com cada um dos monstros
                                                {
                                                    if(CheckCollisionRecs((Rectangle){jogador.corpo.x + QUADRADO, jogador.corpo.y, QUADRADO, QUADRADO}, monstros[j].corpo)) //Teste de colis�o com um monstro
                                                    {//Zera a posi��o e tamanho do monstro e aumenta a pontua��o do jogador
														ZeraMonstros(&monstros[j].corpo);
                                                        jogador.pontuacao += PONTOS_MONSTRO; //Pontua��o recebe valor
                                                        jogador.pontuacaoFaseAtual += PONTOS_MONSTRO;
                                                        PlaySound(monstrosMorte);
                                                    }
                                                }
                                                break;

                                            case 'e': //Caso esquerda
                                                DrawTexture(jogador.soco, jogador.corpo.x - QUADRADO, jogador.corpo.y, WHITE);//Desenho do soco a esquerda de Jogador
                                                if(posicoesDaTela[((int)jogador.corpo.y - HUD) / QUADRADO][((int)jogador.corpo.x - QUADRADO) / QUADRADO] == 'B')
                                                {//Teste se a posi��o a esquerda do jogador � um bloco quebr�vel
                                                    posicoesDaTela[((int)jogador.corpo.y - HUD) / QUADRADO][((int)jogador.corpo.x - QUADRADO) / QUADRADO] = 'C'; //Transforma bloco em colet�vel
                                                    PlaySound(quebrarBlocos);
                                                }
                                                else if((int)jogador.corpo.x % QUADRADO != 0)
                                                {//Caso o jogador esteja entre 2 posi��es X, testa se a 2 posi��es a esquerda � um bloco quebr�vel
                                                    if(posicoesDaTela[((int)jogador.corpo.y - HUD) / QUADRADO][((int)jogador.corpo.x - (2 * QUADRADO)) / QUADRADO] == 'B')
                                                    {
                                                        posicoesDaTela[((int)jogador.corpo.y - HUD) / QUADRADO][((int)jogador.corpo.x - (2 * QUADRADO)) / QUADRADO] = 'C'; //Transforma bloco em colet�vel
                                                        PlaySound(quebrarBlocos);
                                                    }
                                                }
                                                if((int)jogador.corpo.y % QUADRADO != 0)
                                                {//Caso o jogador esteja entre 2 posi��es Y
                                                    if(posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][((int)jogador.corpo.x - QUADRADO) / QUADRADO] == 'B')
                                                    {//Teste se a posi��o a esquerda da outra posi��o Y que Jogador est� entre � um bloco quebr�vel
                                                        posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][((int)jogador.corpo.x - QUADRADO) / QUADRADO] = 'C'; //Transforma bloco em colet�vel
                                                    }
                                                    else if((int)jogador.corpo.x % QUADRADO != 0)
                                                    {//Caso ainda Jogador esteja entre 2 posi��es X e 2 posi��es Y, testa a se 2 posi��es a esquerda da outra posi��o Y � quebr�vel
                                                        if(posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][((int)jogador.corpo.x - (2 * QUADRADO)) / QUADRADO] == 'B')
                                                        {
                                                            posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][((int)jogador.corpo.x - (2 * QUADRADO)) / QUADRADO] = 'C'; //Transforma bloco em colet�vel
                                                            PlaySound(quebrarBlocos);
                                                        }
                                                    }
                                                }
                                                for(j = 0; j < monstrosNoMapa; j++)//Verifica a colis�o com cada um dos monstros
                                                {
                                                    if(CheckCollisionRecs((Rectangle){jogador.corpo.x - QUADRADO, jogador.corpo.y, QUADRADO, QUADRADO}, monstros[j].corpo)) //Teste de colis�o com um monstro
                                                    {//Zera a posi��o e tamanho do monstro e aumenta a pontua��o do jogador
														ZeraMonstros(&monstros[j].corpo);
                                                        jogador.pontuacao += PONTOS_MONSTRO; //Pontua��o recebe valor
                                                        jogador.pontuacaoFaseAtual += PONTOS_MONSTRO;
                                                        PlaySound(monstrosMorte);
                                                    }
                                                }
                                                break;

                                            case 'b': //Caso baixo
                                                DrawTextureRec(jogador.soco, (Rectangle){QUADRADO, 0, QUADRADO, QUADRADO}, (Vector2){jogador.corpo.x, jogador.corpo.y + QUADRADO}, WHITE);//Desenho do soco abaixo de Jogador
                                                if(posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][(int)jogador.corpo.x / QUADRADO] == 'B')
                                                {//Teste se a posi��o abaixo do jogador � um bloco quebr�vel
                                                    posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][(int)jogador.corpo.x / QUADRADO] = 'C'; //Transforma bloco em colet�vel
                                                    PlaySound(quebrarBlocos);
                                                }
                                                else if((int)jogador.corpo.y % QUADRADO != 0)
                                                {//Caso o jogador esteja entre 2 posi��es Y, testa se 2 posi��es abaixo � um bloco quebr�vel
                                                    if(posicoesDaTela[((int)jogador.corpo.y - HUD + (2 * QUADRADO)) / QUADRADO][(int)jogador.corpo.x / QUADRADO] == 'B')
                                                    {
                                                        posicoesDaTela[((int)jogador.corpo.y - HUD + (2 * QUADRADO)) / QUADRADO][(int)jogador.corpo.x / QUADRADO] = 'C'; //Transforma bloco em colet�vel
                                                        PlaySound(quebrarBlocos);
                                                    }
                                                }
                                                if((int)jogador.corpo.x % QUADRADO != 0)
                                                {//Caso o jogador esteja entre 2 posi��es X
                                                    if(posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][((int)jogador.corpo.x + QUADRADO) / QUADRADO] == 'B')
                                                    {//Teste se a posi��o abaixo da outra posi��o X que Jogador est� entre � um bloco quebr�vel
                                                        posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][((int)jogador.corpo.x + QUADRADO)/ QUADRADO] = 'C'; //Transforma bloco em colet�vel
                                                        PlaySound(quebrarBlocos);
                                                    }
                                                    else if((int)jogador.corpo.y % QUADRADO != 0)
                                                    {//Caso ainda Jogador esteja entre 2 posi��es X e 2 posi��es Y, testa a se 2 posi��es abaixo da outra posi��o X � quebr�vel
                                                        if(posicoesDaTela[((int)jogador.corpo.y - HUD + (2 * QUADRADO)) / QUADRADO][((int)jogador.corpo.x + QUADRADO) / QUADRADO] == 'B')
                                                        {
                                                            posicoesDaTela[((int)jogador.corpo.y - HUD + (2 * QUADRADO)) / QUADRADO][((int)jogador.corpo.x + QUADRADO) / QUADRADO] = 'C'; //Transforma bloco em colet�vel
                                                            PlaySound(quebrarBlocos);
                                                        }
                                                    }
                                                }
                                                for(j = 0; j < monstrosNoMapa; j++)//Verifica a colis�o com cada um dos monstros
                                                {
                                                    if(CheckCollisionRecs((Rectangle){jogador.corpo.x, jogador.corpo.y + QUADRADO, QUADRADO, QUADRADO}, monstros[j].corpo))//Teste de colis�o com um monstro
                                                    {//Zera a posi��o e tamanho do monstro e aumenta a pontua��o do jogador
														ZeraMonstros(&monstros[j].corpo);
                                                        jogador.pontuacao += PONTOS_MONSTRO; //Pontua��o recebe valor
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
                                        {//Testa se o jogador tem coletaveis para usar o poder (ou esta no modo Deus) e se o numero de objetos arremessados atualmente � menor que o maximo
                                            PlaySound(jogador.arramessarObjeto);
                                            jogador.objetosArremessados++; //Come�a um novo arremesso
                                            k = 0; //Serve para iniciar as posi��es do novo arremesso
                                            if(jogador.modoDeus == 0)
                                            {//Se o modo Deus estiver ativado n�o ser�o descontados os coletaveis e objetos ser�o gratuitos
                                                jogador.coletaveis -= COLETAVEIS_PARA_OBJETO * dificuldade; //Desconta coletaveis para o arremesso
                                            }
                                        }
                                        else
                                        {
                                            PlaySound(jogador.falhaObjeto); //Som que indica que n�o � poss�vel
                                        }
                                    }
                                    if(jogador.objetosArremessados > 0)
                                    {//Caso ao menos um objeto esteja sendo arremessado
                                        for(j = 0; j < MAX_OBJETOS; j++)
                                        {//Teste cada um dos objetos
                                            if(jogador.sentidoObjetos[j] == 's' && k == 0)
                                            {//Testa se ele ainda n�o tem dire��o e se k = 0, para ent�o iniciar os objetos
                                                jogador.sentidoObjetos[j] = jogador.sentido; //O sentido do objeto � o mesmo do jogador
                                                switch(jogador.sentidoObjetos[j])
                                                {//Ser�o iniciadas as posi��es e dimens�es do objeto relativos a posi��o do jogador para a dire��o que o jogador est�
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
                                                switch(jogador.sentidoObjetos[j]) //Atualiza a posi��o na tela do objeto baseado no sentido e verifica colis�es
                                                {
                                                    case 'd': //Caso direita
                                                        jogador.objetos[j].x += VELOCIDADE_OBJETO; //Desloca o objeto com a velocidade dele
                                                        //Testes de colis�o com blocos quebr�veis, parades e armadilhas
                                                        if(posicoesDaTela[((int)jogador.objetos[j].y - HUD) / QUADRADO][((int)jogador.objetos[j].x + VELOCIDADE_OBJETO) / QUADRADO] == 'B')
                                                        {//Caso a posi��o for um bloco quebr�vel
                                                            posicoesDaTela[((int)jogador.objetos[j].y - HUD) / QUADRADO][((int)jogador.objetos[j].x + VELOCIDADE_OBJETO) / QUADRADO] = 'C'; //Substitui por uma moeda
                                                            PlaySound(quebrarBlocos);
                                                            ZeraObjetos(&jogador.objetos[j], &jogador.objetosArremessados, &jogador.sentidoObjetos[j]);//Zera o objeto o fazendo sumir ap�s a colis�o
                                                        }
                                                        else if(posicoesDaTela[((int)jogador.objetos[j].y - HUD  + (QUADRADO / 2)) / QUADRADO][((int)jogador.objetos[j].x + VELOCIDADE_OBJETO) / QUADRADO] == 'B')
                                                        {//Caso a posi��o for um bloco quebr�vel
                                                            posicoesDaTela[((int)jogador.objetos[j].y - HUD + (QUADRADO / 2)) / QUADRADO][((int)jogador.objetos[j].x + VELOCIDADE_OBJETO) / QUADRADO] = 'C';//Substitui por uma moeda
                                                            PlaySound(quebrarBlocos);
                                                            ZeraObjetos(&jogador.objetos[j], &jogador.objetosArremessados, &jogador.sentidoObjetos[j]);//Zera o objeto o fazendo sumir ap�s a colis�o
                                                        }
                                                        else if(posicoesDaTela[((int)jogador.objetos[j].y - HUD) / QUADRADO][((int)jogador.objetos[j].x + VELOCIDADE_OBJETO) / QUADRADO] == '#' ||  posicoesDaTela[((int)jogador.objetos[j].y - HUD) / QUADRADO][((int)jogador.objetos[j].x + VELOCIDADE_OBJETO) / QUADRADO] == 'X' || posicoesDaTela[((int)jogador.objetos[j].y - HUD + (QUADRADO / 2)) / QUADRADO][((int)jogador.objetos[j].x + VELOCIDADE_OBJETO) / QUADRADO] == '#' ||  posicoesDaTela[((int)jogador.objetos[j].y - HUD + (QUADRADO / 2)) / QUADRADO][((int)jogador.objetos[j].x + VELOCIDADE_OBJETO) / QUADRADO] == 'X')
                                                        {
                                                            ZeraObjetos(&jogador.objetos[j], &jogador.objetosArremessados, &jogador.sentidoObjetos[j]);//Zera o objeto o fazendo sumir ap�s a colis�o
                                                        }

                                                        break;
                                                    case 'e': //Caso esquerda
                                                        jogador.objetos[j].x -= VELOCIDADE_OBJETO;//Desloca o objeto com a velocidade dele
                                                        //Testes de colis�o com blocos quebr�veis, parades e armadilhas
                                                        if(posicoesDaTela[((int)jogador.objetos[j].y - HUD) / QUADRADO][((int)jogador.objetos[j].x) / QUADRADO] == 'B')
                                                        {//Caso a posi��o for um bloco quebr�vel
                                                            posicoesDaTela[((int)jogador.objetos[j].y - HUD) / QUADRADO][((int)jogador.objetos[j].x) / QUADRADO] = 'C';//Substitui por uma moeda
                                                            PlaySound(quebrarBlocos);
                                                            ZeraObjetos(&jogador.objetos[j], &jogador.objetosArremessados, &jogador.sentidoObjetos[j]);//Zera o objeto o fazendo sumir ap�s a colis�o
                                                        }
                                                        else if(posicoesDaTela[((int)jogador.objetos[j].y - HUD  + (QUADRADO / 2)) / QUADRADO][((int)jogador.objetos[j].x) / QUADRADO] == 'B')
                                                        {//Caso a posi��o for um bloco quebr�vel
                                                            posicoesDaTela[((int)jogador.objetos[j].y - HUD + (QUADRADO / 2)) / QUADRADO][((int)jogador.objetos[j].x) / QUADRADO] = 'C';//Substitui por uma moeda
                                                            PlaySound(quebrarBlocos);
                                                            ZeraObjetos(&jogador.objetos[j], &jogador.objetosArremessados, &jogador.sentidoObjetos[j]);//Zera o objeto o fazendo sumir ap�s a colis�o
                                                        }
                                                        else if(posicoesDaTela[((int)jogador.objetos[j].y - HUD) / QUADRADO][((int)jogador.objetos[j].x) / QUADRADO] == '#' ||  posicoesDaTela[((int)jogador.objetos[j].y - HUD) / QUADRADO][((int)jogador.objetos[j].x) / QUADRADO] == 'X' || posicoesDaTela[((int)jogador.objetos[j].y - HUD + (QUADRADO / 2)) / QUADRADO][((int)jogador.objetos[j].x) / QUADRADO] == '#' ||  posicoesDaTela[((int)jogador.objetos[j].y - HUD + (QUADRADO / 2)) / QUADRADO][((int)jogador.objetos[j].x) / QUADRADO] == 'X')
                                                        {
                                                            ZeraObjetos(&jogador.objetos[j], &jogador.objetosArremessados, &jogador.sentidoObjetos[j]);//Zera o objeto o fazendo sumir ap�s a colis�o
                                                        }
                                                        break;
                                                    case 'b': //Caso baixo
                                                        jogador.objetos[j].y += VELOCIDADE_OBJETO;//Desloca o objeto com a velocidade dele
                                                        //Testes de colis�o com blocos quebr�veis, parades e armadilhas
                                                        if(posicoesDaTela[((int)jogador.objetos[j].y - HUD + VELOCIDADE_OBJETO) / QUADRADO][((int)jogador.objetos[j].x) / QUADRADO] == 'B')
                                                        {//Caso a posi��o for um bloco quebr�vel
                                                            posicoesDaTela[((int)jogador.objetos[j].y - HUD  + VELOCIDADE_OBJETO) / QUADRADO][((int)jogador.objetos[j].x) / QUADRADO] = 'C';//Substitui por uma moeda
                                                            PlaySound(quebrarBlocos);
                                                            ZeraObjetos(&jogador.objetos[j], &jogador.objetosArremessados, &jogador.sentidoObjetos[j]);//Zera o objeto o fazendo sumir ap�s a colis�o
                                                        }
                                                        else if(posicoesDaTela[((int)jogador.objetos[j].y - HUD + VELOCIDADE_OBJETO) / QUADRADO][((int)jogador.objetos[j].x + (QUADRADO / 2)) / QUADRADO] == 'B')
                                                        {//Caso a posi��o for um bloco quebr�vel
                                                            posicoesDaTela[((int)jogador.objetos[j].y - HUD + VELOCIDADE_OBJETO) / QUADRADO][((int)jogador.objetos[j].x + (QUADRADO / 2)) / QUADRADO] = 'C';//Substitui por uma moeda
                                                            PlaySound(quebrarBlocos);
                                                            ZeraObjetos(&jogador.objetos[j], &jogador.objetosArremessados, &jogador.sentidoObjetos[j]);//Zera o objeto o fazendo sumir ap�s a colis�o
                                                        }
                                                        else if(posicoesDaTela[((int)jogador.objetos[j].y - HUD + VELOCIDADE_OBJETO) / QUADRADO][((int)jogador.objetos[j].x) / QUADRADO] == '#' ||  posicoesDaTela[((int)jogador.objetos[j].y - HUD + VELOCIDADE_OBJETO) / QUADRADO][((int)jogador.objetos[j].x) / QUADRADO] == 'X' || posicoesDaTela[((int)jogador.objetos[j].y - HUD + VELOCIDADE_OBJETO) / QUADRADO][((int)jogador.objetos[j].x + (QUADRADO / 2)) / QUADRADO] == '#' ||  posicoesDaTela[((int)jogador.objetos[j].y - HUD + VELOCIDADE_OBJETO) / QUADRADO][((int)jogador.objetos[j].x + (QUADRADO / 2)) / QUADRADO] == 'X')
                                                        {
                                                            ZeraObjetos(&jogador.objetos[j], &jogador.objetosArremessados, &jogador.sentidoObjetos[j]);//Zera o objeto o fazendo sumir ap�s a colis�o
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
                                        {//Testa se houve colis�o com espinhos na posi��o corrigida do jogador
                                            jogador.vida--; //Reinicia o mapa
                                            if(dificuldade == 1)//Se a dificuldade for f�cil perde uma vida
                                            {
                                                jogador.vidaTotal--;
                                            }
                                            else//Se fpr dif�cil o jogador perde todas vidas imediatamente
                                            {
                                                jogador.vidaTotal = 0;
                                            }
                                            //Pontua��o e rel�quia na fase reiniciadas e pontua��o diminuida por uma taxa adicional
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
                                        {//Testa se houve colis�o com espinhos na posi��o corrigida do jogador
                                            jogador.vida--;//Reinicia o mapa
                                            if(dificuldade == 1)//Se a dificuldade for f�cil perde uma vida
                                            {
                                                jogador.vidaTotal--;
                                            }
                                            else//Se fpr dif�cil o jogador perde todas vidas imediatamente
                                            {
                                                jogador.vidaTotal = 0;
                                            }
                                            //Pontua��o e rel�quia na fase reiniciadas e pontua��o diminuida por uma taxa adicional
                                            jogador.pontuacao -= (jogador.pontuacaoFaseAtual + 100);//Transformar em fun��o PerderVida
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
                                        {//Testa se houve colis�o com espinhos na posi��o corrigida do jogador
                                            jogador.vida--;//Reinicia o mapa
                                            if(dificuldade == 1)//Se a dificuldade for f�cil perde uma vida
                                            {
                                                jogador.vidaTotal--;
                                            }
                                            else//Se fpr dif�cil o jogador perde todas vidas imediatamente
                                            {
                                                jogador.vidaTotal = 0;
                                            }
                                            //Pontua��o e rel�quia na fase reiniciadas e pontua��o diminuida por uma taxa adicional
                                            jogador.pontuacao -= (jogador.pontuacaoFaseAtual + 100);//Transformar em fun��o PerderVida
                                            if(jogador.reliquiaFaseAtual == 1)
                                            {
                                                jogador.reliquias--;
                                            }
                                            PlaySound(jogador.morte);
                                        }
                                    }
                                    else if(jogador.vida == 1 && posicoesDaTela[((int)jogador.corpo.y - HUD) / QUADRADO][(int)jogador.corpo.x / QUADRADO] == 'X' || posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][(int)jogador.corpo.x / QUADRADO] == 'X' || posicoesDaTela[((int)jogador.corpo.y - HUD - jogador.velocidadePulo) / QUADRADO][(int)jogador.corpo.x / QUADRADO] == 'X')
                                    {//Testa a colis�o na posi��o exata de jogador, tanto quanto se os p�s do jogador ou a cabe�a est�o encostando em um espinho na sua borda
                                        jogador.vida--;//Reinicia o mapa
                                        if(dificuldade == 1)//Se a dificuldade for f�cil perde uma vida
                                        {
                                            jogador.vidaTotal--;
                                        }
                                        else//Se fpr dif�cil o jogador perde todas vidas imediatamente
                                        {
                                            jogador.vidaTotal = 0;
                                        }
                                        //Pontua��o e rel�quia na fase reiniciadas e pontua��o diminuida por uma taxa adicional
                                        jogador.pontuacao -= (jogador.pontuacaoFaseAtual + 100);//Transformar em fun��o PerderVida
                                        if(jogador.reliquiaFaseAtual == 1)
                                        {
                                            jogador.reliquias--;
                                        }
                                        PlaySound(jogador.morte);
                                    }

                                    //Moedas (diversos ifs s�o necess�rios para permitir pegar mais de uma moeda por vez
                                    if((int)jogador.corpo.x % QUADRADO != 0 && (int)jogador.corpo.y % QUADRADO != 0)
                                    {//Caso o jogador esteja entre 2 quadrados em  X e Y
                                        if(posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][((int)jogador.corpo.x + QUADRADO) / QUADRADO] == 'C')
                                        {//Testa se houve colis�o com uma moeda na posi��o corrigida do jogador
                                            ColetaColetavel(&jogador.coletaveis, &jogador.pontuacaoFaseAtual, &jogador.pontuacao, coletarColetavel); //Incrementa a contagem de colet�veis e a pontua��o do jogador
                                            posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][((int)jogador.corpo.x + QUADRADO) / QUADRADO] = ' '; //Torna o espa�o vazio
                                        }
                                    }
                                    if((int)jogador.corpo.y % QUADRADO != 0)
                                    {//Caso o jogador esteja entre 2 quadrados em  Y
                                        if(posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][(int)jogador.corpo.x / QUADRADO] == 'C')
                                        {//Testa se houve colis�o com uma moeda na posi��o corrigida do jogador
                                            ColetaColetavel(&jogador.coletaveis, &jogador.pontuacaoFaseAtual, &jogador.pontuacao, coletarColetavel); //Incrementa a contagem de colet�veis e a pontua��o do jogador
                                            posicoesDaTela[((int)jogador.corpo.y - HUD + QUADRADO) / QUADRADO][(int)jogador.corpo.x / QUADRADO] = ' '; //Torna o espa�o vazio
                                        }
                                    }
                                    if((int)jogador.corpo.x % QUADRADO != 0)
                                    {//Caso o jogador esteja entre 2 quadrados em  X
                                        if(posicoesDaTela[((int)jogador.corpo.y - HUD) / QUADRADO][((int)jogador.corpo.x + QUADRADO) / QUADRADO] == 'C')
                                        {//Testa se houve colis�o com uma moeda na posi��o corrigida do jogador
                                            ColetaColetavel(&jogador.coletaveis, &jogador.pontuacaoFaseAtual, &jogador.pontuacao, coletarColetavel); //Incrementa a contagem de colet�veis e a pontua��o do jogador
                                            posicoesDaTela[((int)jogador.corpo.y - HUD) / QUADRADO][((int)jogador.corpo.x + QUADRADO) / QUADRADO] = ' '; //Torna o espa�o vazio
                                        }
                                    }
                                    if(posicoesDaTela[((int)jogador.corpo.y - HUD) / QUADRADO][(int)jogador.corpo.x / QUADRADO] == 'C')
                                    {//Caso a posi��o exata do jogador seja uma moeda
                                        ColetaColetavel(&jogador.coletaveis, &jogador.pontuacaoFaseAtual, &jogador.pontuacao, coletarColetavel); //Incrementa a contagem de colet�veis e a pontua��o do jogador
                                        posicoesDaTela[((int)jogador.corpo.y - HUD) / QUADRADO][(int)jogador.corpo.x / QUADRADO] = ' '; //Torna o espa�o vazio
                                    }

                                    //Portal
                                    if(CheckCollisionRecs(jogador.corpo, (Rectangle){portal.pos.x, portal.pos.y, QUADRADO, QUADRADO}))
                                    {//Como s� tem um portal, � mais eficiente comparar a colis�o do ret�ngulo do jogador com o do portal
                                        jogador.faseAtual++; //Incrementa a fase atual, indicando que passou de fase
                                        sprintf(mapaAtual, "mapa%d.txt\0", jogador.faseAtual); //Prepara a string do nome do mapa da fase seguinte
                                        jogador.vida--; //Reinica a fase atualizando para o mapa seguinte
                                        jogador.vidaTotal++; //B�nus de 1 de vida a cada mapa completo
                                        jogador.pontuacao += 250; //B�nus de 250 de pontua��o por fase completa
                                        EndDrawing();
                                        BeginDrawing();
                                        ClearBackground(RAYWHITE);
                                        //Final do jogo (vit�ria!)
                                        if(jogador.reliquias == numeroDeMapas)
                                        {//Testa se o n�mero de rel�quias � igual ao n�mero de mapas, indicando que o jogador terminou o jogo e coletou todas
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
                                        {//Testa se o jogador passou do �ltimo mapa previsto, completando o jogo
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
                                        {//Se o jogo ainda n�o tiver sido ganho exibe a tela de sucesso e continua
                                            DrawTexture(levelComplete, 0, 0, WHITE);
                                            EndDrawing();
                                            PlaySound(somLevelComplete);
                                            WaitTime(3);
                                        }
                                        EndDrawing();
                                    }

                                    //Reliquias
                                    if(CheckCollisionRecs(jogador.corpo, reliquia.pos))//Testa se a rel�quia � coletada
                                    {//Como s� tem uma rel�quia, � mais eficiente comparar a colis�o do ret�ngulo do jogador com o da reliquia
                                        jogador.reliquias++; //Incrementa a contagem de rel�quias
                                        jogador.reliquiaFaseAtual = 1; //Indica que a rel�quia da fase foi coletada
                                        jogador.pontuacao += 300; //Incrementa a pontua��o do jogador
                                        jogador.pontuacaoFaseAtual += 300;
                                        //Zera o ret�ngulo da rel�quia para que ela suma
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
                                        jogador.pontuacao -= (jogador.pontuacaoFaseAtual + 100); //Reinicia a pontua��o da fase e diminui um valor extra
                                        if(jogador.reliquiaFaseAtual == 1) //Retira a rel�quia caso ela tenha sido coletada
                                        {
                                            jogador.reliquias--;
                                        }
                                    }
                                    if(jogador.pontuacao < 0) //Evita que o jogador fique com menos de 0 de pontua��o
                                    {
                                        jogador.pontuacao = 0; //A pontua��o minima � 0
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
                                    if(jogador.modoDeus == 1)//Mostra a vida como 999 caso o modo Deus esteja ativado independente da dificuldade, pois a vida no modo � infinita
                                    {
                                        DrawText("Vidas: 999", LARGURA * 28 / 64, ALTURA - (HUD / 2), 25, BLACK);
                                    }
                                    else if(dificuldade == 1)//Mostra a vida caso a dificuldade seja f�cil
                                    {
                                        DrawText(TextFormat(("Vidas: %d"), jogador.vidaTotal), LARGURA * 28 / 64, ALTURA - (HUD / 2), 25, BLACK);
                                    }
                                    else//Mostra a vida como 1 caso a dificuldade seja dif�cil, pois qualquer colis�o com inimigos ou espinhos resulta em morte
                                    {
                                        DrawText("Vidas: 1", LARGURA * 28 / 64, ALTURA - (HUD / 2), 25, BLACK);
                                    }
                                    DrawTexture(jogador.objeto, LARGURA * 141 / 256, ALTURA - (HUD / 2) + 5, WHITE);
                                    //Objetos dispon�veis
                                    if(jogador.coletaveis < COLETAVEIS_PARA_OBJETO * dificuldade) //0 se o jogador n�o possuir o m�nimo para um objeto
                                    {
                                        DrawText("Objetos Disponiveis: 0", LARGURA * 72 / 128, ALTURA - (HUD / 2), 25, BLACK);
                                    }
                                    else if(jogador.coletaveis < COLETAVEIS_PARA_OBJETO * dificuldade * 2)//Para todos os seguintes, mostra o n�mero dispon�vel pelo n�mero de colet�veis menos o n�mero de objetos arremessados
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
										//Adicionar uma parte que l� o nome do jogador e armazena sua pontua��o
                                        remove(saves.atual); //Remove o arquivo do save atual
                                        //Para a m�sica
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
                                        opcaoPause = 1; //Come�a o loop de pause com a op��o 1 selecionada
                                        pause = 1; //Mant�m o loop de pause
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
                                            {//Cada case equivale a uma op��o e eles s�o acessados com enter. Um ret�ngulo mais escuro � desenhado por cima do selecionado
                                                case 1://Op��o Continuar Jogo
                                                    DrawRectangle(LARGURA / 8, ALTURA * 1 / 10, LARGURA * 6 / 8, ALTURA / 12, DARKBLUE);
                                                    if(IsKeyPressed(KEY_ENTER))
                                                    {
                                                        PlaySound(selecionar);
                                                        pause = 0; //Quebra o loop de pause
                                                    }
                                                    break;
                                                case 2://Op��o Salvar Jogo
                                                    DrawRectangle(LARGURA / 8 , ALTURA * 2.5 / 10, LARGURA * 6 / 8, ALTURA / 12, DARKBLUE);
                                                    if(IsKeyPressed(KEY_ENTER))
                                                    {
                                                        PlaySound(selecionar);
                                                        savesJogo = fopen(saves.atual, "w");//Abre o save com o n�mero desejado e escreve as informa��es do novo estado de jogo
                                                        for(j = 0; j < LINHAS; j++)
                                                        {
                                                            for(k = 0; k < COLUNAS; k++)
                                                            {
                                                                putc(posicoesDaTela[j][k], savesJogo); //Coloca cada informa��o de cada posi��o da matriz do mapa no arquivo (n�o leg�vel pois n�o inclui \n
                                                            }
                                                        }
                                                        fprintf(savesJogo, "\n"); //\n para n�o causar erros na hora da leitura
                                                        fprintf(savesJogo, "%d %d %d %d %d\n", monstrosNoMapa, dificuldade, musica, contadorTextura, jogador.modoDeus); //Salva algumas informa��es quanto a op��es n�o atreladas a nada e alguns valores que precisam ser lidos antes posteriormente
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
                                                case 3://Op��o Config
                                                    DrawRectangle(LARGURA / 8, ALTURA * 4 / 10, LARGURA * 6 / 8, ALTURA / 12, DARKBLUE);
                                                    if(IsKeyPressed(KEY_ENTER))
                                                    {
                                                        if(IsKeyPressed(KEY_ENTER))
                                                        {
                                                            PlaySound(selecionar);
                                                            PlaySound(config);
                                                            Config(intrasport, &musica, controles, introducao, selecionar, &dificuldade, &jogador.vida, &jogador.modoDeus); //Abre o menu de configura��es
                                                        }
                                                    }
                                                    break;
                                                case 4://Op��o Skins N�O IMPLEMENTADO A TEMPO INFELIZMENTE, N�O FAZ NADA
                                                    DrawRectangle(LARGURA / 8, ALTURA * 5.5 / 10, LARGURA * 6 / 8, ALTURA / 12, DARKBLUE);
                                                    if(IsKeyPressed(KEY_ENTER))
                                                    {
                                                        PlaySound(selecionar);
                                                        PlaySound(skins);
                                                    }
                                                    break;
                                                case 5://Op��o Menu Principal
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

                                                case 6: //Op��o Sair Do Jogo
                                                    DrawRectangle(LARGURA / 8, ALTURA * 8.5 / 10, LARGURA * 6 / 8, ALTURA / 12, DARKBLUE);
                                                    if(IsKeyPressed(KEY_ENTER))
                                                    {
                                                        PlaySound(sair);
                                                        PlaySound(selecionar);
                                                        //Quebram todos la�os at� sair do jogo
                                                        pause = 0;
                                                        jogador.vida = 0;
                                                        comeco = 0;
                                                        continuarMenu = 0;
                                                        WaitTime(2); //Tempo para mensagem de despedida ser tocado
                                                        fechar = 1; //Faz o loop principal ser quebrado e o jogo se fechar
                                                    }
                                                    break;
                                            }
                                            //Desenhos dos textos das op��es
                                            DrawText("Continuar Jogo", LARGURA / 2 - (MeasureText("Continuar Jogo", 60) / 2), ALTURA * 1 / 10, 60, WHITE);
                                            DrawText("Salvar", LARGURA / 2 - (MeasureText("Salvar", 60) / 2), ALTURA * 2.5 / 10, 60, WHITE);
                                            DrawText("Config", LARGURA / 2 - (MeasureText("Config", 60) / 2), ALTURA * 4 / 10, 60, WHITE);
                                            DrawText("Skins", LARGURA / 2 - (MeasureText("Skins", 60) / 2), ALTURA * 5.5 / 10, 60, WHITE);
                                            DrawText("Menu Principal", LARGURA / 2 - (MeasureText("Menu Principal", 60) / 2), ALTURA * 7 / 10, 60, WHITE);
                                            DrawText("Sair Do Jogo", LARGURA / 2 - (MeasureText("Sair Do Jogo", 60) / 2), ALTURA * 8.5 / 10, 60, WHITE);

                                            if(IsKeyPressed(KEY_ESCAPE))
                                            {
                                                pause = 1; //Clicar esc tamb�m quebra o la�o e sai do menu de pause
                                            }
                                            SelecionadorMenus(&opcaoPause, 6);
                                            EndDrawing();
                                        }
                                    }
                                }
                            }

                        }//Fim da parte da gameplay
                        //Retornar
                        else if(IsKeyPressed(KEY_ENTER) && saves.opcao == 4 && partida == 0)//Caso nenhum save seja selecionado e sim a op��o de retorno
                        {
                            continuarMenu = 0; //Quebra o la�o da tela de saves e retorna para o menu principal
                            PlaySound(voltar);
                            PlaySound(selecionar);

                        }
                        EndDrawing();
                    }

                }
                break;
			//--------------------------------------------------------------------------------------
            //Configura��es
            //--------------------------------------------------------------------------------------
            case 3:
                DrawRectangle(LARGURA / 8, ALTURA * 4.75 / 10, LARGURA * 6 / 8, ALTURA / 12, (Color){80, 3, 6, 255});
                if(IsKeyPressed(KEY_ENTER))
                {
                    PlaySound(selecionar);
                    PlaySound(config);
                    Config(intrasport, &musica, controles, introducao, selecionar, &dificuldade, &jogador.vida, &jogador.modoDeus); //Abre o menu de configura��es
                }
                break;
			//--------------------------------------------------------------------------------------
            //Selecionar Skin
            //--------------------------------------------------------------------------------------
            case 4: //N�O IMPLEMENTADO A TEMPO INFELIZMENTE, N�O FAZ NADA
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
            case 5: //N�O IMPLEMENTADO A TEMPO INFELIZMENTE, N�O FAZ NADA
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
        //Textos das op��es do menu principal
        DrawText("Novo Jogo", LARGURA / 2 - (MeasureText("Novo Jogo", 60) / 2), ALTURA * 1.75 / 10, 60, WHITE);
        DrawText("Continuar", LARGURA / 2 - (MeasureText("Continuar", 60) / 2), ALTURA * 3.25 / 10, 60, WHITE);
        DrawText("Config", LARGURA / 2 - (MeasureText("Config", 60) / 2), ALTURA * 4.75 / 10, 60, WHITE);
        DrawText("Skins", LARGURA / 2 - (MeasureText("Skins", 60) / 2), ALTURA * 6.25 / 10, 60, WHITE);
        DrawText("Placar", LARGURA / 2 - (MeasureText("Placar", 60) / 2), ALTURA * 7.75 / 10, 60, WHITE);
        DrawText("Sair", LARGURA / 2 - (MeasureText("Sair", 60) / 2), ALTURA * 9.25 / 10, 60, WHITE);
        //Mudar op��o marcada
        SelecionadorMenus(&opcao, 6);
        Musica(intrasport, musica);
        EndDrawing();
    }
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
