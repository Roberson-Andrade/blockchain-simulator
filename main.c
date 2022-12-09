#include "openssl/crypto.h" //arquivo de definição necessário para SHA256_DIGEST_LENGTH
#include "openssl/sha.h"    //arquivo de definição necessário função SHA256
#include <string.h>
#include <stdio.h>
#include "mtwister.c"

int NUMERO_TOTAL_DE_BLOCOS = 100000;

struct BlocoNaoMinerado
{
  unsigned int numero;
  unsigned int nonce;
  unsigned char data[184];
  unsigned char hashAnterior[SHA256_DIGEST_LENGTH];
};
typedef struct BlocoNaoMinerado BlocoNaoMinerado;

struct BlocoMinerado
{
  BlocoNaoMinerado bloco;
  unsigned char hash[SHA256_DIGEST_LENGTH];
  struct BlocoMinerado *prox;
};
typedef struct BlocoMinerado BlocoMinerado;

struct BlocoOrdenado
{
  int totalBitcoin;
  unsigned char hashAnterior[SHA256_DIGEST_LENGTH];
  BlocoMinerado *enderecoBlocoMinerado;
  struct BlocoOrdenado *prox;
};

typedef struct BlocoOrdenado BlocoOrdenado;

void imprimeHash(unsigned char hash[], int length);
void minerarBloco();
void calculaTransacoes(BlocoNaoMinerado *blocoAMinerar, MTRand *randNumber);
void preencheHashAnterior(BlocoNaoMinerado *blocoAMinerar);
void mineraBlocos(BlocoMinerado **primeiroBloco);
void validaBloco(BlocoNaoMinerado *blocoAMinerar, BlocoMinerado **primeiroBloco);
void insereBlocoMinerado(BlocoMinerado **primeiroBloco, BlocoNaoMinerado blocoRecemMinerado, unsigned char hash[]);
void buscaBloco(BlocoMinerado *primeiroBloco, int numeroDoBloco);
int somaTotalBitcoin(BlocoMinerado *primeiroBloco);
void ordenaBlocoEmOrdemCrescente(BlocoMinerado *primeiroBloco, BlocoOrdenado **primeiroBlocoOrdenado);
void insereBlocoOrdenado(BlocoMinerado *blocoAtual, BlocoOrdenado **primeiroBlocoOrdenado, int totalBitcoinAtual);
void insereBlocoOrdenadoNoInicio(BlocoOrdenado **primeiroBlocoOrdenado, int totalBitcoinAtual, BlocoMinerado *blocoAtual);
void insereBlocoOrdenadoDepois(BlocoOrdenado *blocoOrdenadoAtual, int totalBitcoinAtual, BlocoMinerado *blocoAtual);
void insereBlocoOrdenadoFim(BlocoOrdenado *ultimoBloco, int totalBitcoinAtual, BlocoMinerado *blocoAtual);
void imprimeBlocosOrdenadosCrescente(BlocoOrdenado *primeiroBlocoOrdenado);
void imprimeBlocosOrdenadosDecrescente(BlocoOrdenado *primeiroBlocoOrdenado);
void imprimeBlocos(BlocoMinerado *primeiroBloco);

int main()
{
  BlocoMinerado *primeiroBloco = NULL;
  BlocoOrdenado *primeiroBlocoOrdenado = NULL;

  mineraBlocos(&primeiroBloco);
  ordenaBlocoEmOrdemCrescente(primeiroBloco, &primeiroBlocoOrdenado);

  int escolha;

  do
  {
    printf("\n[1] Buscar o hash pelo numero do bloco.\n");
    printf("[2] Listar os blocos por total de bitcoin em ordem crescente.\n");
    printf("[3] Listar os blocos por total de bitcoin em ordem decrescente.\n");
    printf("[0] Sair\n\n");
    scanf("%d", &escolha);

    switch (escolha)
    {
    case 1:
    {
      int numeroEscolhido;
      printf("Digite um numero de 1 a 100.000.\n");
      scanf("%d", &numeroEscolhido);
      buscaBloco(primeiroBloco, numeroEscolhido);
      break;
    }

    case 2:
    {
      imprimeBlocosOrdenadosCrescente(primeiroBlocoOrdenado);
      break;
    }

    case 3:
    {
      imprimeBlocosOrdenadosDecrescente(primeiroBlocoOrdenado);
      break;
    }

    case 0:
      break;

    default:
      printf("Opcao invalida!\n");
    }

  } while (escolha != 0);
  return 0;
}

/**
 * @brief Função que minera todos os blocos
 *
 * @param primeiroBloco
 */
void mineraBlocos(BlocoMinerado **primeiroBloco)
{

  BlocoNaoMinerado blocoAMinerar;
  MTRand randNumber = seedRand(1234567);

  for (int i = 0; i < NUMERO_TOTAL_DE_BLOCOS; i++)
  {
    blocoAMinerar.numero = i + 1;

    if (blocoAMinerar.numero == 1)
    {
      for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
      {
        blocoAMinerar.hashAnterior[i] = 0;
      }
    }

    calculaTransacoes(&blocoAMinerar, &randNumber);
    validaBloco(&blocoAMinerar, primeiroBloco);
  }
}

/**
 * @brief Simula dados das transações para gerar o hash
 *
 * @param blocoAMinerar
 * @param randNumber
 */
void calculaTransacoes(BlocoNaoMinerado *blocoAMinerar, MTRand *randNumber)
{

  unsigned char qtdTransacoes = (unsigned char)(1 + (genRandLong(randNumber) % 61));

  for (int i = 0; i < qtdTransacoes; i++)
  {
    unsigned char endOrigem = (unsigned char)genRandLong(randNumber) % 256;
    unsigned char endDst = (unsigned char)genRandLong(randNumber) % 256;
    unsigned char qtdBitcoin = (unsigned char)(1 + genRandLong(randNumber) % 50);

    int dataPosition = i * 3;

    blocoAMinerar->data[dataPosition] = endOrigem;
    blocoAMinerar->data[dataPosition + 1] = endDst;
    blocoAMinerar->data[dataPosition + 2] = qtdBitcoin;
  }

  for (int i = qtdTransacoes * 3; i < 184; i++)
  {
    blocoAMinerar->data[i] = 0;
  }
}

/**
 * @brief Função que varia o nounce para encontrar os 2 primeiros zeros e validar o hash
 *
 * @param blocoAMinerar
 * @param primeiroBloco
 */
void validaBloco(BlocoNaoMinerado *blocoAMinerar, BlocoMinerado **primeiroBloco)
{
  unsigned char hash[SHA256_DIGEST_LENGTH];

  int nounceCount = 0;

  do
  {
    blocoAMinerar->nonce = nounceCount;

    SHA256((unsigned char *)blocoAMinerar, sizeof(blocoAMinerar), hash);

    nounceCount++;
  } while (hash[0] != 0 || hash[1] != 0);

  insereBlocoMinerado(primeiroBloco, *blocoAMinerar, hash);
}

/**
 * @brief Insere o bloco recém minerado na lista encadeada
 *
 * @param primeiroBloco
 * @param blocoRecemMinerado
 * @param hash
 */
void insereBlocoMinerado(BlocoMinerado **primeiroBloco, BlocoNaoMinerado blocoRecemMinerado, unsigned char hash[])
{
  BlocoMinerado *novoBloco = malloc(sizeof(BlocoMinerado));

  novoBloco->bloco = blocoRecemMinerado;

  for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
  {
    novoBloco->hash[i] = hash[i];
  }

  if (*primeiroBloco == NULL)
  {
    novoBloco->prox = NULL;

    *primeiroBloco = novoBloco;
    return;
  }

  for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
  {
    novoBloco->bloco.hashAnterior[i] = (*primeiroBloco)->hash[i];
  }

  novoBloco->prox = *primeiroBloco;
  *primeiroBloco = novoBloco;
}

/**
 * @brief Imprime na tela um hash
 *
 * @param hash
 * @param length
 */
void imprimeHash(unsigned char hash[], int length)
{
  int i;

  for (i = 0; i < length; ++i)
    printf("%02x", hash[i]);

  printf("\n");
}

/**
 * @brief Imprime todos os blocos minerados em tela
 *
 * @param primeiroBloco
 */
void imprimeBlocos(BlocoMinerado *primeiroBloco)
{

  if (primeiroBloco == NULL)
  {
    return;
  }

  imprimeBlocos(primeiroBloco->prox);
  printf("Bloco: %d hash: ", primeiroBloco->bloco.numero);
  imprimeHash(primeiroBloco->hash, SHA256_DIGEST_LENGTH);
}

/**
 * @brief Imprime blocos ordenados em forma crescente
 *
 * @param primeiroBlocoOrdenado
 */
void imprimeBlocosOrdenadosCrescente(BlocoOrdenado *primeiroBlocoOrdenado)
{

  if (primeiroBlocoOrdenado == NULL)
  {
    return;
  }

  printf("Bloco: %d bitcoin: %d\n", primeiroBlocoOrdenado->totalBitcoin, primeiroBlocoOrdenado->totalBitcoin);
  imprimeBlocosOrdenadosCrescente(primeiroBlocoOrdenado->prox);
}

/**
 * @brief Imprime blocos ordenados em forma crescente
 *
 * @param primeiroBlocoOrdenado
 */
void imprimeBlocosOrdenadosDecrescente(BlocoOrdenado *primeiroBlocoOrdenado)
{

  if (primeiroBlocoOrdenado == NULL)
  {
    return;
  }

  imprimeBlocosOrdenadosDecrescente(primeiroBlocoOrdenado->prox);
  printf("Endereco: %p bitcoin: %d\n", primeiroBlocoOrdenado->enderecoBlocoMinerado, primeiroBlocoOrdenado->totalBitcoin);
}

/**
 * @brief Busca um bloco na lista encadeada pelo seu número com recursividade
 *
 * @param primeiroBloco
 * @param numeroDoBloco
 */
void buscaBloco(BlocoMinerado *primeiroBloco, int numeroDoBloco)
{
  if (primeiroBloco == NULL)
  {
    printf("\n Bloco nao encontrado \n");
    return;
  }

  if (primeiroBloco->bloco.numero == numeroDoBloco)
  {
    printf("O bloco numero %d tem hash igual a ", numeroDoBloco);
    imprimeHash(primeiroBloco->hash, SHA256_DIGEST_LENGTH);
    printf("\n");
    return;
  }

  buscaBloco(primeiroBloco->prox, numeroDoBloco);
}

/**
 * @brief Calcula e retorna o numero total de bitcoins de um determinado bloco minerado
 *
 * @param primeiroBloco
 * @return int
 */
int somaTotalBitcoin(BlocoMinerado *primeiroBloco)
{
  int total = 0;

  int index = 0;

  while (primeiroBloco->bloco.data[index] != 0)
  {
    int dataPosition = index * 3;

    total += primeiroBloco->bloco.data[dataPosition + 2];

    index += 1;
  }

  return total;
}

/**
 * @brief Cria uma nova lista encadeada ordenada a partir dos blocos minerados
 *
 * @param primeiroBloco
 * @param primeiroBlocoOrdenado
 */
void ordenaBlocoEmOrdemCrescente(BlocoMinerado *primeiroBloco, BlocoOrdenado **primeiroBlocoOrdenado)
{

  if (primeiroBloco == NULL)
  {
    return;
  }

  int totalBitcoin = somaTotalBitcoin(primeiroBloco);

  insereBlocoOrdenado(primeiroBloco, primeiroBlocoOrdenado, totalBitcoin);

  ordenaBlocoEmOrdemCrescente(primeiroBloco->prox, primeiroBlocoOrdenado);
}

/**
 * @brief Insere novo bloco ordenado no início
 *
 * @param primeiroBlocoOrdenado
 * @param totalBitcoinAtual
 * @param blocoAtual
 */
void insereBlocoOrdenadoNoInicio(BlocoOrdenado **primeiroBlocoOrdenado, int totalBitcoinAtual, BlocoMinerado *blocoAtual)
{
  BlocoOrdenado *novoBlocoOrdenado = malloc(sizeof(BlocoOrdenado));
  novoBlocoOrdenado->prox = *primeiroBlocoOrdenado;
  novoBlocoOrdenado->totalBitcoin = totalBitcoinAtual;
  novoBlocoOrdenado->enderecoBlocoMinerado = blocoAtual;
  *primeiroBlocoOrdenado = novoBlocoOrdenado;
}

/**
 * @brief Insere bloco ordenado depois de qualquer bloco indicado na lista
 *
 * @param blocoOrdenadoAtual
 * @param totalBitcoinAtual
 * @param blocoAtual
 */
void insereBlocoOrdenadoDepois(BlocoOrdenado *blocoOrdenadoAtual, int totalBitcoinAtual, BlocoMinerado *blocoAtual)
{
  BlocoOrdenado *novoBlocoOrdenado = malloc(sizeof(BlocoOrdenado));
  novoBlocoOrdenado->prox = blocoOrdenadoAtual->prox;
  novoBlocoOrdenado->totalBitcoin = totalBitcoinAtual;
  novoBlocoOrdenado->enderecoBlocoMinerado = blocoAtual;

  blocoOrdenadoAtual->prox = novoBlocoOrdenado;
}

/**
 * @brief Insere bloco ordenado no fim
 *
 * @param ultimoBloco
 * @param totalBitcoinAtual
 * @param blocoAtual
 */
void insereBlocoOrdenadoFim(BlocoOrdenado *ultimoBloco, int totalBitcoinAtual, BlocoMinerado *blocoAtual)
{
  BlocoOrdenado *novoBlocoOrdenado = malloc(sizeof(BlocoOrdenado));
  novoBlocoOrdenado->prox = NULL;
  novoBlocoOrdenado->totalBitcoin = totalBitcoinAtual;
  novoBlocoOrdenado->enderecoBlocoMinerado = blocoAtual;

  ultimoBloco->prox = novoBlocoOrdenado;
}

void insereBlocoOrdenado(BlocoMinerado *blocoAtual, BlocoOrdenado **primeiroBlocoOrdenado, int totalBitcoinAtual)
{

  if (*primeiroBlocoOrdenado == NULL || totalBitcoinAtual <= (*primeiroBlocoOrdenado)->totalBitcoin)
  {
    insereBlocoOrdenadoNoInicio(primeiroBlocoOrdenado, totalBitcoinAtual, blocoAtual);
    return;
  }

  BlocoOrdenado *blocoAuxiliar = *primeiroBlocoOrdenado;

  while (blocoAuxiliar->prox != NULL)
  {
    if (totalBitcoinAtual <= blocoAuxiliar->prox->totalBitcoin)
    {
      insereBlocoOrdenadoDepois(blocoAuxiliar, totalBitcoinAtual, blocoAtual);
      return;
    }

    blocoAuxiliar = blocoAuxiliar->prox;
  }

  insereBlocoOrdenadoFim(blocoAuxiliar, totalBitcoinAtual, blocoAtual);
}