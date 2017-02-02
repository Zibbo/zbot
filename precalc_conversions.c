#include <poker-eval/poker_defs.h>
#include <poker-eval/inlines/eval.h>

#include "defs.h"
#include "precalc_conversions.h"



int cards_to_int_4[52][52][52][52];
int cards_to_int_3[52][52][52];
int cards_to_int_2[52][52];
int cards_to_int_1[52];
int cards_to_int_2_nosuit[13][13];

//struct cards_4 int_to_cards_4[270725];

struct cards_4 int_to_cards_4[270725];
struct cards_3 int_to_cards_3[22100];
struct cards_2 int_to_cards_2[1326];
struct cards_1 int_to_cards_1[52];
struct cards_2 int_to_cards_2_nosuit[78];

CardMask int_to_cardmask_4[270725];
CardMask int_to_cardmask_3[22100];
CardMask int_to_cardmask_2[1326];

int preflop_morph_mapping[1326];



void precalc_conversions()
{
  int c = 0, i, j, k, l;
  CardMask tmpmask1, tmpmask2, tmpmask3;

  //printf("func addr: %lx\n", &precalc_conversions);
  c = 0;
  for (i = 51; i >=0;i--)
    {
      for (j = i-1;j >= 0; j--)
        {
          for (k = j-1;k >= 0; k--)
            {
	      for (l = k-1;l >= 0; l--)
		{
		  cards_to_int_4[i][j][k][l] = c;
		  cards_to_int_4[i][j][l][k] = c;
		  cards_to_int_4[i][l][j][k] = c;
		  cards_to_int_4[i][k][j][l] = c;
		  cards_to_int_4[i][k][l][j] = c;
		  cards_to_int_4[i][l][k][j] = c;

		  cards_to_int_4[j][i][k][l] = c;
                  cards_to_int_4[j][i][l][k] = c;
                  cards_to_int_4[j][l][i][k] = c;
                  cards_to_int_4[j][k][i][l] = c;
                  cards_to_int_4[j][k][l][i] = c;
                  cards_to_int_4[j][l][k][i] = c;

		  cards_to_int_4[k][j][i][l] = c;
                  cards_to_int_4[k][j][l][i] = c;
                  cards_to_int_4[k][l][j][i] = c;
                  cards_to_int_4[k][i][j][l] = c;
                  cards_to_int_4[k][i][l][j] = c;
                  cards_to_int_4[k][l][i][j] = c;

		  cards_to_int_4[l][j][k][i] = c;
                  cards_to_int_4[l][j][i][k] = c;
                  cards_to_int_4[l][i][j][k] = c;
                  cards_to_int_4[l][k][j][i] = c;
                  cards_to_int_4[l][k][i][j] = c;
                  cards_to_int_4[l][i][k][j] = c;
		  
		  int_to_cards_4[c].c1 = i;
		  int_to_cards_4[c].c2 = j;
		  int_to_cards_4[c].c3 = k;
		  int_to_cards_4[c].c4 = l;
		  
		  CardMask_RESET(int_to_cardmask_4[c]);
		  CardMask_SET(int_to_cardmask_4[c], i);
		  CardMask_SET(int_to_cardmask_4[c], j);
		  CardMask_SET(int_to_cardmask_4[c], k);
		  CardMask_SET(int_to_cardmask_4[c], l);

		  c++;
		}
	    }
	}
    }
  c = 0;

  for (i = 51; i >=0;i--)
    {
      for (j = i-1;j >= 0; j--)
	{
	  for (k = j-1;k >= 0; k--)
	    {
	      cards_to_int_3[i][j][k] = c;
	      cards_to_int_3[i][k][j] = c;
	      cards_to_int_3[k][i][j] = c;
	      cards_to_int_3[j][i][k] = c;
	      cards_to_int_3[j][k][i] = c;
	      cards_to_int_3[k][j][i] = c;

	      
	      int_to_cards_3[c].c1 = i;
	      int_to_cards_3[c].c2 = j;
	      int_to_cards_3[c].c3 = k;

	      //cards_to_int_3[i][j][k] = c;
	      
	      tmpmask1 = Deck_MASK(i);
	      tmpmask2 = Deck_MASK(j);
	      tmpmask3 = Deck_MASK(k);
	      CardMask_RESET(int_to_cardmask_3[c]);
	      CardMask_SET(int_to_cardmask_3[c], i);
	      CardMask_SET(int_to_cardmask_3[c], j);
	      CardMask_SET(int_to_cardmask_3[c], k);

	      c++;
	    }
	}
    }
  
  c = 0;
  for (i = 51; i >=0;i--)
    {
      for (j = i-1;j >= 0; j--)
        {
          cards_to_int_2[i][j] = c;
	  cards_to_int_2[j][i] = c;


	  int_to_cards_2[c].c1 = i;
	  int_to_cards_2[c].c2 = j;
	  
	  cards_to_int_2[i][j] = c;

	  tmpmask1 = Deck_MASK(i);
	  tmpmask2 = Deck_MASK(j);

	  CardMask_RESET(int_to_cardmask_2[c]);
	  CardMask_SET(int_to_cardmask_2[c], i);
	  CardMask_SET(int_to_cardmask_2[c], j);

	  c++;
	}
    }

  c = 0;
  for (i = 51; i >=0;i--)
    {
      cards_to_int_1[i] = c;
      cards_to_int_1[i] = c;
      int_to_cards_1[c].c1 = i;

      c++;
    }
  c = 0;
  for (i = 12; i >=0;i--)
    {
      for (j = i-1;j >= 0; j--)
        {
	  cards_to_int_2_nosuit[i][j] = c;
	  cards_to_int_2_nosuit[j][i] = c;
	  int_to_cards_2_nosuit[c].c1 = i;
	  int_to_cards_2_nosuit[c].c2 = j;
	  c++;
	}
    }
  c = 0;
  for (i = 51; i >=0;i--)
    {
      for (j = i-1;j >= 0; j--)
        {
	  if (i%13 == j%13)
	    preflop_morph_mapping[c] = i%13;
	  else if (i/13 == j/13)
	    preflop_morph_mapping[c] = 13+cards_to_int_2_nosuit[i%13][j%13];
	  else
	    preflop_morph_mapping[c] = 13+78+cards_to_int_2_nosuit[i%13][j%13];
	  c++;
	}
    }
}
