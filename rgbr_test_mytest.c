/* rgbr_test.c
 * Mike Johanson, Nov11, 02010
 * johanson@ualberta.ca
 *
 * A simple test program that calls dlopen to load an RGBR agent
 * and tries querying its action probabilities.  Used to help test
 * RGBR agents before using them in the actual real game best response
 * program.
 */

/* C includes */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <dlfcn.h>

/* RGBR includes */
#include "rgbr_interface.h"

int main( int argc, char *argv[] )
{
  /* Check that the right number of parameters were passed */
  if( argc != 3 ) {
    fprintf( stderr, "Usage: ./rgbr_test <.so file> <arg string>\n" );
    return 1;
  }

  /* Load the shared object */
  rgbr_agent_t rgbr_agent;
  char *lib = argv[ 1 ];
  rgbr_agent.arg_string = argv[ 2 ];
  
  if( ( rgbr_agent.dl_obj = dlopen( lib, RTLD_LAZY ) ) == 0 ) {
    char *error_msg;
    error_msg = dlerror();
    if( error_msg ) {
      fprintf( stderr, "dl_error on dlopen: [%s]\n", error_msg );
    }
    return 1;
  }
  /* Load the 2 required functions */
  if( ( rgbr_agent.init_private_info = 
	dlsym( rgbr_agent.dl_obj, "rgbr_init_private_info" ) ) == 0 ) {
    fprintf( stderr, "Could not find rgbr_init_private_info\n" );
    dlclose( rgbr_agent.dl_obj );
    return 1;
  }
  if( ( rgbr_agent.get_action_probs = 
	dlsym( rgbr_agent.dl_obj, "rgbr_get_action_probs" ) ) == 0 ) {
    fprintf( stderr, "Could not find rgbr_get_action_probs\n" );
    dlclose( rgbr_agent.dl_obj );
    return 1;
  }
  
  /* Call the library's init function */
  rgbr_agent.private_info = 
    rgbr_agent.init_private_info( rgbr_agent.arg_string );
  if( rgbr_agent.private_info == NULL ) {
    fprintf( stderr, "init_private_info failed with argument [%s]\n", 
	     rgbr_agent.arg_string );
    return 1;
  }


  int round = 0;
  int board_cards[ 5 ];
  int num_actions[ 4 ];
  int actions[ 4 ][ 6 ];
  int num_private_hands;
  int private_hands[ 1326 ][ 2 ];
  double probs[ 1326 ][ 3 ];

  /* Try calling get_action_probs at the start of the game */
  round = 3;
  num_actions[ 0 ] = 2;
  num_actions[ 1 ] = 2;
  num_actions[ 2 ] = 2;
  num_actions[ 3 ] = 0;

  actions[0][0] = 2;
  actions[0][1] = 1;
  actions[1][0] = 1;
  actions[1][1] = 1;
  actions[2][0] = 1;
  actions[2][1] = 1;

  board_cards[0] = 31;
  board_cards[1] = 35;
  board_cards[2] = 46;
  board_cards[3] = 50;
  board_cards[4] = 41;


  int x, y;
  /* Iterate over possible hands such that smaller card comes first */
  num_private_hands = 0;
  for( x = 0; x < 52; x++ ) {
    for( y = x+1; y < 52; y++ ) {
      private_hands[ num_private_hands ][ 0 ] = x;
      private_hands[ num_private_hands ][ 1 ] = y;
      num_private_hands++;
    }
  }
  
  num_private_hands = 29;

  int error = rgbr_agent.get_action_probs( rgbr_agent.private_info,
					    round,
					    board_cards,
					    num_actions,
					    actions,
					    num_private_hands,
					    private_hands,
					    probs );
  if( error == 1 ) {
    fprintf( stderr, "Error on get_action_probs\n" );
    return 1;
  }
  /* Print out a test hand */
  printf( "Action probs test:\n" );
  int z;
  for( z = 0; z < num_private_hands; z++) {
    printf( "  %s%s: [%lg,%lg,%lg] = %lg\n", 
	    rgbr_card_to_string[ private_hands[ z ][ 0 ] ],
	    rgbr_card_to_string[ private_hands[ z ][ 1 ] ],
	    probs[ z ][ 0 ],
	    probs[ z ][ 1 ],
	    probs[ z ][ 2 ],
	    probs[ z ][ 0 ] + probs[ z ][ 1 ] + probs[ z ][ 2 ] );
  }



  /* Close the library */
  dlclose( rgbr_agent.dl_obj );
  
  return 0;
}
