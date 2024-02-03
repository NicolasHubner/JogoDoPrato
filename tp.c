#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
//#include <stdlib.h>

#define NUM_PRATOS 8
#define NUM_SUPORTES 8

#define FPS 60

#define SCREEN_W 960
#define SCREEN_H 540

//largura do jogador
#define JOGADOR_W 50
#define JOGADOR_H 100

#define SUPORTES_OFFSET_X 100
#define SUPORTE_WIDTH 5

#define PRATO_WIDTH 100

#define SUPORTES_OFFSET_Y (SCREEN_H / 2 - JOGADOR_H / 2)
#define SUPORTES_SPACING ((SCREEN_W - 2 * SUPORTES_OFFSET_X - NUM_SUPORTES * SUPORTE_WIDTH) / (NUM_SUPORTES - 1))

bool isHighScore = false;
int highScore = 0;

typedef struct Jogador {
	
	float x;
	int equilibrando;
	int mov_esq, mov_dir;
	ALLEGRO_COLOR cor;
	float vel;
	
} Jogador;

typedef struct Prato {
	float x;
	float y;
	/* um valor entre 0 e 255, em que 0 = prato equilibrado e
	   1 = prato com maxima energia, prestes a cair */
	float energia;
	float tempoParaAparecer;
    ALLEGRO_COLOR cor;
    int selecionado;
} Prato;

int seq[] = {3, 4, 5, 2, 7, 6, 0, 1};

void desenha_cenario() {
    al_clear_to_color(al_map_rgb(0, 122, 122));
}

void desenha_jogador(Jogador j) {
    al_draw_filled_triangle(j.x, SCREEN_H - JOGADOR_H,
        j.x - JOGADOR_W / 2, SCREEN_H,
        j.x + JOGADOR_W / 2, SCREEN_H,
        j.cor);
}

void atualizaJogador(Jogador *j) {
	if(j->mov_esq) {
		if(j->x - j->vel > JOGADOR_W / 2)
			j->x -= j->vel;
	}
	if(j->mov_dir) {
		if(j->x + j->vel < SCREEN_W - JOGADOR_W / 2)
			j->x += j->vel;
	}	
}


void InicializaJogador(Jogador *j) {
	j->x = SCREEN_W / 2;\
	j->equilibrando = 0;
	j->cor = al_map_rgb(0, 223, 15);
	j->mov_esq = 0;
	j->mov_dir = 0;
	j->vel = 4;
}

float geraTempoPrato(int i) {
	return 1;
}

void desenha_prato(Prato p, int suporteIndex) {
    float pratoX = p.x - PRATO_WIDTH / 2;
    int pratoHeight = 10;
    float pratoY = SUPORTES_OFFSET_Y - 162;

    int r = 255 * p.energia;
    int g = 255 * (1 - p.energia);

    ALLEGRO_COLOR corPrato = al_map_rgb(r, g, 1);

    float supX = SUPORTES_OFFSET_X + seq[suporteIndex] * (SUPORTES_SPACING + SUPORTE_WIDTH);


    al_draw_filled_rectangle(supX - PRATO_WIDTH / 2, pratoY, supX + PRATO_WIDTH / 2, pratoY + pratoHeight, corPrato);

}


void inicializaPratos(Prato pratos[], float supHeight) {
    float intervaloPratos = 5.0; // Intervalo de 3 segundos entre os pratos
    float tempoAparecimento = 0.0;

    for (int i = 0; i < NUM_PRATOS; i++) {
        pratos[i].x = SUPORTES_OFFSET_X + (seq[i]) * (SUPORTES_SPACING + SUPORTE_WIDTH);
        pratos[i].y = SUPORTES_OFFSET_Y + supHeight - 20;

        pratos[i].tempoParaAparecer = tempoAparecimento;
        pratos[i].energia = 0;
        pratos[i].cor = al_map_rgb(255, 255, 0); // Cor inicial (amarelo)
        pratos[i].selecionado = 0;
        tempoAparecimento += intervaloPratos;
    }
}

void atualizaPratos(Prato pratos[], int tempoDecorrido, int *playing) {
    int i;
    for (i = 0; i < NUM_PRATOS; i++) {
        float tempoParaAparecer = pratos[i].tempoParaAparecer * 1000; // Convertendo para milissegundos
        // if(pratos[i].energia > 0){
        // }
        // if(pratos[seq[i]].energia > 0 ){
        // printf("Prato %d do suporte %d\n", i, seq[i]);
        // }
        if (tempoDecorrido >= tempoParaAparecer) {
        // printf("Prato %d do suporte %d, com energia %f e tempo %f está selecionado? %d\n", i, seq[i], pratos[i].energia, pratos[i].tempoParaAparecer, pratos[seq[i]].selecionado);
        // printf("\n");

            float tempoDecorridoPrato = tempoDecorrido - tempoParaAparecer;
            float progresso = tempoDecorridoPrato / 15000; // Progresso em relação a 15 segundos
            if(pratos[seq[i]].selecionado == 1 && pratos[i].energia >= 0.009){
                // printf("prato energia: %f\n", pratos[i].energia);
                pratos[i].energia -= 0.008; // Atualiza a energia do prato de 0 a 1
            } else {
                if (pratos[i].energia < 1.0) {
                    pratos[i].energia += 0.0007; // Atualiza a energia do prato de 0 a 1
                    // pratos[i].cor = al_map_rgb(255 * (1 - progresso), 255 * progresso, 0); // Interpolação de cores
                } else {
                    pratos[i].energia = 1.0; // Energia máxima do prato
                    pratos[i].cor = al_map_rgb(255, 0, 0); // Cor máxima (vermelho)
                    printf("F PRATO %d\n", i);
                    *playing = 2;
                }
            }
        }
    }
}

void criar_suportes(Prato pratos[]) {
    float supWidth = SUPORTE_WIDTH;
    float supHeight = (3 * SCREEN_H) / 4;
    ALLEGRO_COLOR colors[] = {
        al_map_rgb(255, 123, 245),
        al_map_rgb(255, 0, 0),
        al_map_rgb(255, 255, 0),
        al_map_rgb(0, 255, 0),
    };

    float supY = (SCREEN_H - supHeight) / 2;

    // Calcula o espaçamento entre os suportes
    float supSpacing = (SCREEN_W - supWidth * NUM_SUPORTES) / (NUM_SUPORTES + 1);

    for (int i = 0; i < NUM_SUPORTES; i++) {
        float supX = supSpacing + (supWidth + supSpacing) * i;

        if(pratos[i].selecionado == 1){
            // printf("Prato %d selecionado\n", seq[i]);
            al_draw_filled_rectangle(supX, supY, supX + supWidth, supY + supHeight, colors[1 * rand() % 4]);
        } else {
        // Desenhar o suporte na posição (supX, supY) com largura supWidth e altura supHeight
        al_draw_filled_rectangle(supX, supY, supX + supWidth, supY + supHeight, al_map_rgb(255, 123, 245));
        }
    }
}

void desenha_pontuacao(int pontos) {
    // Define a cor e o tamanho da fonte

    // printf("Pontuacao: %d\n", pontos);
    ALLEGRO_FONT* fonte = al_create_builtin_font();
    ALLEGRO_COLOR corTexto = al_map_rgb(255, 255, 255);
    int tamanhoTexto = 20;

    // Converte a pontuação para uma string
    char strPontuacao[30];
    sprintf(strPontuacao, "Pontuacao: %d", pontos);

    // Obtém a largura e altura do texto
    int larguraTexto = al_get_text_width(fonte, strPontuacao);
    int alturaTexto = al_get_font_line_height(fonte);

    // Calcula a posição para desenhar o texto
    int posicaoX = SCREEN_W - larguraTexto - 10; // 10 pixels de margem à direita
    int posicaoY = 10;

    // Desenha o texto da pontuação na tela
    al_draw_text(fonte, corTexto, posicaoX, posicaoY, ALLEGRO_ALIGN_LEFT, strPontuacao);

    // Libera a memória da fonte
    al_destroy_font(fonte);
}


int main(int argc, char **argv){
	
	ALLEGRO_DISPLAY *display = NULL;	
	ALLEGRO_TIMER *timer = NULL;
	ALLEGRO_EVENT_QUEUE *event_queue = NULL;
	ALLEGRO_EVENT ev;
	
	//inicializa o Allegro
	if(!al_init()) {
		fprintf(stderr, "failed to initialize allegro!\n");
		return -1;
	}
	
	//cria um temporizador que incrementa uma unidade a cada 1.0/FPS segundos
    timer = al_create_timer(1.0 / FPS);
    if(!timer) {
		fprintf(stderr, "failed to create timer!\n");
		return -1;
	}	
	
    //inicializa o módulo de primitivas do Allegro
    if(!al_init_primitives_addon()){
		fprintf(stderr, "failed to initialize primitives!\n");
        return -1;
    }	
	
	//inicializa o modulo que permite carregar imagens no jogo
	if(!al_init_image_addon()){
		fprintf(stderr, "failed to initialize image module!\n");
		return -1;
	}	
	
	//cria uma tela com dimensoes de SCREEN_W, SCREEN_H pixels
	display = al_create_display(SCREEN_W, SCREEN_H);
	if(!display) {
		fprintf(stderr, "failed to create display!\n");
		al_destroy_timer(timer);
		return -1;
	}	
	
	//instala o teclado
	if(!al_install_keyboard()) {
		fprintf(stderr, "failed to install keyboard!\n");
		return -1;
	}
	
	//inicializa o modulo allegro que carrega as fontes
	al_init_font_addon();

	//inicializa o modulo allegro que entende arquivos tff de fontes
	if(!al_init_ttf_addon()) {
		fprintf(stderr, "failed to load tff font module!\n");
		return -1;
	}
	
	//carrega o arquivo arial.ttf da fonte Arial e define que sera usado o tamanho 32 (segundo parametro)
    ALLEGRO_FONT *size_32 = al_load_font("arial.ttf", 32, 1);   
	if(size_32 == NULL) {
		fprintf(stderr, "font file does not exist or cannot be accessed!\n");
	}	
	
	
 	//cria a fila de eventos
	event_queue = al_create_event_queue();
	if(!event_queue) {
		fprintf(stderr, "failed to create event_queue!\n");
		al_destroy_display(display);
		al_destroy_timer(timer);
		return -1;
	}	
	
	//registra na fila os eventos de tela (ex: clicar no X na janela)
	al_register_event_source(event_queue, al_get_display_event_source(display));
	//registra na fila os eventos de tempo: quando o tempo altera de t para t+1
	al_register_event_source(event_queue, al_get_timer_event_source(timer));
	//registra na fila os eventos de teclado (ex: pressionar uma tecla)
	al_register_event_source(event_queue, al_get_keyboard_event_source()); 	
	
	
	//JOGADOR
	Jogador jogador;
	InicializaJogador(&jogador);
	
	//PRATOS
	Prato pratos[NUM_PRATOS];
	inicializaPratos(pratos, JOGADOR_H);
	
    int pontuacao = 0;
	int playing=1;
	
	//inicia o temporizador
	al_start_timer(timer);

    // int seq[] = {4, 5, 6, 2, 7, 3, 0, 1};
	
	while(playing == 1) {
		int tempoDecorrido = al_get_time() * 1000;
		//espera por um evento e o armazena na variavel de evento ev
		al_wait_for_event(event_queue, &ev);
		
        if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            switch (ev.keyboard.keycode) {
                case ALLEGRO_KEY_LEFT:
                case ALLEGRO_KEY_A:
                    jogador.mov_esq = 1;
                    for (int i = 0; i < NUM_SUPORTES; i++) {
                        pratos[i].selecionado = 0;
                    }
                    break;
                case ALLEGRO_KEY_RIGHT:
                case ALLEGRO_KEY_D:
                    jogador.mov_dir = 1;
                    for (int i = 0; i < NUM_SUPORTES; i++) {
                        pratos[i].selecionado = 0;
                    }
                    break;
                case ALLEGRO_KEY_SPACE:
                    for (int i = 0; i < NUM_SUPORTES; i++) {
                    float supX = SUPORTES_OFFSET_X + i * (SUPORTES_SPACING + SUPORTE_WIDTH);
                    //Abranger maior area, facilitar pro jogador
                    if (jogador.x >= (supX - 10) && jogador.x <= supX + SUPORTE_WIDTH + 10) {
                        // int numSuporte = seq[i];
                        pratos[i].selecionado = 1;
                        // printf("Prato %d selecionado, seq de %d\n", i, seq[i]);
                    }
                }
            }
        }

        if (ev.type == ALLEGRO_EVENT_KEY_UP) {
            switch (ev.keyboard.keycode) {
                case ALLEGRO_KEY_LEFT:
                case ALLEGRO_KEY_A:
                    jogador.mov_esq = 0;
                    break;
                case ALLEGRO_KEY_RIGHT:
                case ALLEGRO_KEY_D:
                    jogador.mov_dir = 0;
                    break;
                case ALLEGRO_KEY_SPACE:
                    for (int i = 0; i < NUM_SUPORTES; i++) {
                    float supX = SUPORTES_OFFSET_X + i * (SUPORTES_SPACING + SUPORTE_WIDTH);
                    //Abranger maior area, facilitar pro jogador
                    if (jogador.x >= (supX - 20) && jogador.x <= supX + SUPORTE_WIDTH + 20) {
                        // int numSuporte = seq[i];
                        pratos[i].selecionado = 0;
                        // printf("Prato %d selecionado\n", i);
                    }
                }
            }
        }

		//se o tipo de evento for um evento do temporizador, ou seja, se o tempo passou de t para t+1
		if(ev.type == ALLEGRO_EVENT_TIMER) {
            tempoDecorrido += 1000 / FPS;
		
			desenha_cenario();
			
			atualizaJogador(&jogador);
            criar_suportes(pratos);


			desenha_jogador(jogador);

            pontuacao = tempoDecorrido / 1000; // Atualiza a pontuação com base no tempo decorrido em segundos
            desenha_pontuacao(pontuacao);

            for (int i = 0; i < NUM_PRATOS; i++) {
                if (tempoDecorrido >= pratos[i].tempoParaAparecer * 1000) {
                    desenha_prato(pratos[i], i);
                }
            }
            atualizaPratos(pratos, tempoDecorrido, &playing);

			//atualiza a tela (quando houver algo para mostrar)
			al_flip_display();
		}
		//se o tipo de evento for o fechamento da tela (clique no x da janela)
		if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			playing = 0;
        }
	}

    ALLEGRO_COLOR corDeFundo = al_map_rgb(0, 0, 0);
    FILE *arquivo = fopen("recorde.txt", "r");
    int highScore = 0;

    if (arquivo != NULL) {
        int resultado = fscanf(arquivo, "%d", &highScore);
        if (resultado == EOF) {
            printf("Arquivo vazio\n");
        }
        fclose(arquivo);
    } else {
        printf("Arquivo não existe\n");
    }

    while (playing == 2) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(event_queue, &ev); // Aguarda um evento

        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            playing = 0; // Sai do loop e encerra o programa
        } else {
            char strPontuacao[30];
            char strHighScore[50];

            al_clear_to_color(corDeFundo);
            al_draw_text(size_32, al_map_rgb(255, 255, 255), SCREEN_W / 2, SCREEN_H / 2, ALLEGRO_ALIGN_CENTRE, "GAME OVER");

            if (pontuacao < highScore) {
                sprintf(strPontuacao, "Pontuação: %d", pontuacao);
                sprintf(strHighScore, "Parabéns, porém o é RECORDE: %d", highScore);
                al_draw_text(size_32, al_map_rgb(255, 255, 255), SCREEN_W / 2, SCREEN_H / 2 + 50, ALLEGRO_ALIGN_CENTRE, strPontuacao);
                al_draw_text(size_32, al_map_rgb(255, 255, 255), SCREEN_W / 2, SCREEN_H / 2 + 100, ALLEGRO_ALIGN_CENTRE, strHighScore);
            }

            if(pontuacao == highScore){
                sprintf(strHighScore, "PARABÉNS, VOCÊ IGUALOU O RECORDE: %d", pontuacao);
                al_draw_text(size_32, al_map_rgb(255, 255, 255), SCREEN_W / 2, SCREEN_H / 2 + 50, ALLEGRO_ALIGN_CENTRE, strHighScore);
            }
            if (pontuacao > highScore) {
                sprintf(strHighScore, "PARABÉNS! NOVO RECORDE: %d", pontuacao);

                al_draw_text(size_32, al_map_rgb(255, 255, 255), SCREEN_W / 2, SCREEN_H / 2 + 50, ALLEGRO_ALIGN_CENTRE, strHighScore);

                arquivo = fopen("recorde.txt", "w");
                if (arquivo != NULL) {
                    fprintf(arquivo, "%d", pontuacao);
                    fclose(arquivo);
                } else {
                    printf("Erro ao abrir o arquivo\n");
                }
            }

            al_flip_display();
        }
    }

	al_destroy_timer(timer);
	al_destroy_display(display);
	al_destroy_event_queue(event_queue);

	return 0;
}