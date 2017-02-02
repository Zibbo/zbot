#ifndef _RGBR_INTERFACE_H
#define _RGBR_INTERFACE_H

#include <stdlib.h>
#include <stdio.h>

/* Interface functions for arbitrary Real Game Best Response agents 
 * Mike Johanson, December 12, 02010
 * johanson@ualberta.ca
 */

/* This interface will allow us to compute the value of a 
 * real game best response to your agent, which will tell us how
 * far from a Nash equilibrium it is in the real game of 
 * Heads-Up Limit Texas Hold'em.  To do this, you will have to
 * implement the two functions below, and then compile your
 * agent as a C shared library (internally, your agent can still
 * be implemented in C++).  Our real game best response program
 * will load your shared object with dlopen, call rgbr_init_private_info
 * to allow your agent to load any lookup tables or otherwise
 * initialize itself, and then will walk the game tree with many
 * threads, each calling rgbr_get_action_probs to query your agent's
 * strategy to compute a best response.
 *
 * Computing a real game best response value for Texas Hold'em is
 * a very large computational task that almost seems infeasible.
 * However, by breaking the task into independent subproblems
 * and taking advantage of card isomorphisms, we can greatly
 * reduce the size of the tree that we need to walk, and solve
 * the problem using a cluster of high-end computers.  On our
 * cluster, we can use 3 nodes with 24 processors and 256 gigs of 
 * RAM per node to compute a real game best response value to one
 * of our strategies in just under 22 hours.  The process may be
 * slower to compute the value for your agent, simply because
 * we've been able to write special code for accessing our own strategies,
 * but we can make up for this by simply using more time and more processors.
 *
 * To compute the value, we walk over the tree of
 * public states of the game - just the actions and the board cards.
 * At each public state, we will ask your agent what its action
 * probabilities are with every possible, canonical set of hole cards 
 * that it can reach that state with.  On the walk from the root to the
 * terminal nodes, we pass forward the set of probabilities of
 * your agent reaching the public state with each set of hole cards;
 * on the way back, we return the value of the best response for
 * each of its possible sets of hole cards in that subtree, 
 * and return the value of the maximizing action as we pass through its 
 * choice nodes.  
 *
 * Every subtree in this tree is an independent problem that can be solved
 * by one thread.  We choose to split the problem into subtrees just 
 * after the flop is dealt, resulting in up to 24,570 subproblems, 
 * consisting of a flop, a preflop betting sequence, and a position 
 * for the players.  
 *   (Note - the 24,570 value only considers the 1755 canonical flops;
 *    if we included all possible flops, there would be
 *      (52*51*50/6 = 22100 flops) 
 *      * (7 nonterminal preflop betting sequences)
 *      * (2 positions ) ) 
 *      = 309,400 subproblems).
 *
 * Note that the result of the computation isn't a best response
 * strategy, but instead simply the value of the best response
 * to your agent.  When the program is finished, it produces the
 * expected value of the best-response player in each seat,
 * holding each set of 1326 hole cards.  Since the hole cards are
 * equally likely, the value for the best response in each seat
 * is simply the average of these 1326 values, and the overall
 * value is the average across the seats.
 * At the moment, the best response strategy itself would take far 
 * too much disk for us to store, and so we can only produce the value
 * of the best response in these (1326 * 2) situations.
 *
 * In order for all of this to work, your agent has to have
 * the following properties.  At the moment, we have no workaround
 * if your agent does not have them, and we would not be able to
 * compute a real game best response for you.
 * 
 *   - Written in C or C++.  Our RGBR program (written in C, running 
 *     on 64-bit Linux) will be loading your agent as a shared object.  If
 *     there is demand from people using Java or other languages,
 *     in the future we may look into communicating over sockets as 
 *     the AAAI competition does.  For now, we haven't gone that route
 *     because of the overhead and the difficulty of threading
 *     the computation.
 *
 *   - Thread-safe.  Your implementation of the rgbr_get_action_probs
 *     function must allow for multiple threads to query different
 *     parts of your agent's strategy at the same time.  This is
 *     required because we'll be loading your strategy into RAM
 *     once and using a 24-core machine to walk different sections
 *     of the game tree at the same time.
 * 
 *   - Static.  Your agent cannot change its strategy over time;
 *     identical calls to rgbr_get_action_probs must return the
 *     same result.  If your strategy changes as we query it, then
 *     different orderings of the subtrees we evaluate could
 *     produce different results.
 * 
 *   - Consistent across isomorphic hands.  For example, on the preflop, 
 *     the strategy used for an Ace of spades and an Ace of hearts
 *     must be identical to that used for an Ace of spades and an
 *     Ace of diamonds.  Likewise, the hand AsAh on the flop 2s2h2d
 *     must have exactly the same strategy as the hand AdAc on the flop 2d2c2s.
 *     In order to make the computation feasible, the program
 *     only considers dealing canonical public cards (for example,
 *     it will deal the flop 2s2h2d but not 2s2h2c, 2s2d2c, or 2h2d2c).
 *     One effect of this is that all 3-of-a-suit flops are spades.
 *     To correct for this, we average the values of isomorphic hands as
 *     we return towards the root on our tree walk.  So long as your
 *     agent uses the same strategy for isomorphic hands (AsKs and AcKc
 *     are treated identically before the flop), then this process
 *     will return the correct value for both hands.  If different
 *     strategies are used, then the value returned will not be correct.
 *     Since removing card isomorphisms is a simple, obvious and safe way
 *     to create a smaller abstract game, most of the AAAI competitors
 *     (especially those interested in Nash equilibrium approximations)
 *     are very likely already doing this.
 *
 *   - Fast.  The real game best response program will repeatedly
 *     call your rgbr_get_action_probs function, and even if your
 *     program just does a table lookup to return the result,
 *     this will still likely be the bottleneck of the process.  It is vital
 *     for this function to return nearly instantly.  If your agent
 *     does any nontrivial computation to produce its action probabilities,
 *     then it will likely be too slow for us to compute a real game
 *     best response in a reasonable timeframe.  The good news is that
 *     the computers we will use for this have 256 gigs of RAM each,
 *     and our program has virtually no overhead - you have almost
 *     all of that RAM available for use by your agent.  So, if your
 *     agent uses lookup tables that you normally read from disk with
 *     lseek or memory map with mmap, you should now have enough memory 
 *     available to simply load them directly into RAM.  For our strategies, 
 *     this has meant the difference between solving each of the 24,570 
 *     subgames in 4.5 minutes when we use RAM, and 1.5 hours when 
 *     we use mmap.
 *
 * COMPILING:
 * To compile your code to a shared object for us to use, you will first
 * have to write a small C file that implements the two functions below,
 * connecting the pipes between our RGBR interface and your program's
 * internal interface.
 * 
 * If your program is written in C++, then remember that your implementation
 * of the two functions below should be declared 'extern "C"', so that the
 * function name is left unchanged.
 *
 * Then, on a 64-bit Linux system, you should just be able to compile
 * all of your object files into a shared object with a gcc command
 * such as:
 * gcc -Wall -O3 -fPIC -shared -Wl,--export-dynamic <your files> -o rgbr_YOURNAME.so
 *
 * Once you have compiled your code to a .so file, you can use the suppiled
 * rgbr_test program to do a simple test to make sure that the two functions
 * described below can be loaded.  Just run:
 * ./rgbr_test <your .so file> <your rgbr_init_private_info argument string>
 * 
 * rgbr_test will attempt to load your .so file, find the two functions,
 * initialize your agent with rgbr_init_private_info, and then call 
 * rgbr_get_action_probs to find your strategy's action probabilities
 * for every preflop hand in the first action of the game.  If you can load
 * the .so file and at least that preliminary test produces the correct
 * probabilities, then you are on the right track.  Try changing the rgbr_test
 * program to query for action probabilities in a few other parts of the game
 * tree.  If those queries are also correct, then you should be ready to send
 * in a copy of your .so file and any lookup tables it might access, and we
 * can start the computation for you.
 */

/*************
 * INTERFACE *
 *************/

/* Some constants describing the number of rounds, numbers of board cards,
 * numbers of actions and actions per round, and so on.
 */
#define RGBR_NUM_ROUNDS 4
#define RGBR_MAX_BOARD_CARDS 5
#define RGBR_MAX_ACTIONS_IN_ROUND 6
#define RGBR_MAX_HANDS 1326
#define RGBR_NUM_HOLE_CARDS 2
#define RGBR_NUM_ACTIONS 3
#define RGBR_ACTION_FOLD 0
#define RGBR_ACTION_CALL 1
#define RGBR_ACTION_RAISE 2


/*
 * rgbr_init_private_info
 *
 * Load the agent.  The 'arg' parameter can be any string containing
 * options that you might need to start your agent.  Returns a void* that
 * our program will pass to your agent in calls to rgbr_get_probs.
 * You can use this pointer to point to any data structures or tables you might
 * need for your strategy.
 *
 * If your program encounters a problem in this function, print a 
 * detailed message to stderr and return NULL.  I'll email the message back to
 * you and we'll determine what went wrong.
 */
void *rgbr_init_private_info( char *arg );


/*
 * rgbr_get_action_probs
 * Get the action probabilities for a single public state (betting and
 * board cards) and multiple private states (the agent's hole cards)
 * This function must be:
 *  - Thread-safe.  Many threads will be calling this function at the same time.
 *  - Consistent.  Given the same input, it must always return the same output.
 *  - Fast.
 *
 * Parameters passed:
 *   - private_info: the void* returned by your 
 *                   rgbr_init_private_info function.  Declared const
 *                   since you shouldn't modify *private_info, both
 *                   because of the need to remain thread-safe and
 *                   to make sure that your agent does not change its
 *                   strategy while the program runs.
 *   - round: The current round in the game
 *   - board_cards[RGBR_MAX_BOARD_CARDS]: An array holding up to 5 board cards. 
 *                    The round tells you how many entries to expect
 *                    to be filled in board_cards: 0 on the preflop,
 *                    3 on the flop, 4 on the turn, 5 on the river.
 *                    On the river, entries 0,1,2 are the flop cards,
 *                    3 is the turn, and 4 is the river.
 *   - num_actions[RGBR_NUM_ROUNDS]: The number of actions taken 
 *                    so far in each round.
 *   - actions[RGBR_MAX_ROUNDS][RGBR_MAX_ACTIONS_IN_ROUND]: 
 *                    The actions taken in each round.  Indexed by round,
 *                    then by action; there are at most 6 actions in each
 *                    round, since the longest sequence is
 *                    check-raise-raise-raise-raise-call.  
 *                    0 = fold, 1 = call, 2 = raise.
 *   - num_private_hands: The number of hands the function is asking you
 *                    to act in.  RGBR won't ask about every possible hand;
 *                    instead, it only asks about hands that you could
 *                    reach this state with (the preceeding sequence
 *                    of actions occur with > 0% probability for this hand) and
 *                    it skips over non-canonical hole cards.
 *   - private_hands[RGBR_MAX_HANDS][RGBR_NUM_HOLE_CARDS]: 
 *                     The hands that RGBR is querying for.
 *                     Only entries 0..num_private_hands are used.
 *                     For an entry, the second index gives you
 *                     the two private cards in that hand.  Check
 *                     rgbr_index_to_card below to see the mapping
 *                     from ints to cards.
 *   - probs[RGBR_MAX_HANDS][RGBR_NUM_ACTIONS]: 
 *                     The value returned by your function.  Each entry
 *                     corresponds to the equivalent entry in private_hands.
 *                     For entries i in 0..num_private_hands, your function must
 *                     fill in probs[i] with the probability that your agent
 *                     will fold, call, and raise in this state.  
 *                     Of course, probs[i][0]+probs[i][1]+probs[i][2] should
 *                     sum to 1.  If not, then RGBR will normalize across
 *                     the legal actions, and if no weight is placed on legal
 *                     actions, then probs[i][1] (call) will be set to 1.
 *
 * On success, your function should return 0.
 * On failure, print a detailed message to stderr and return 1.  I'll
 * email the error message back to you and we can investigate what went wrong.
 * This might be a problem if multiple threads return errors at the same
 * time and the lines from the error messages get interleaved; if it becomes
 * an issue, then we'll find a better solution.
 */
int rgbr_get_action_probs( const void *private_info,
			   int round,
			   const int board_cards[ RGBR_MAX_BOARD_CARDS ], 
			   const int num_actions[ RGBR_NUM_ROUNDS ], 
			   int actions[ RGBR_NUM_ROUNDS ][ RGBR_MAX_ACTIONS_IN_ROUND ],
			   const int num_private_hands,
			   int private_hands[ RGBR_MAX_HANDS ][ RGBR_NUM_HOLE_CARDS ],
			   double probs[ RGBR_MAX_HANDS ][ RGBR_NUM_ACTIONS ] );


/*
 * rgbr_index_to_card
 * An array that maps the card indices used in board_cards and
 * private_hands to strings, to illustrate how the indexing works.
 */
const char *rgbr_card_to_string[] = { "2s", // 0
				      "2h", // 1
				      "2d", // 2
				      "2c", // 3
				      "3s", // 4
				      "3h", // 5
				      "3d", // 6
				      "3c", // 7
				      "4s", // 8
				      "4h", // 9
				      "4d", // 10
				      "4c", // 11
				      "5s", // 12
				      "5h", // 13
				      "5d", // 14
				      "5c", // 15
				      "6s", // 16
				      "6h", // 17
				      "6d", // 18
				      "6c", // 19
				      "7s", // 20
				      "7h", // 21
				      "7d", // 22
				      "7c", // 23
				      "8s", // 24
				      "8h", // 25
				      "8d", // 26
				      "8c", // 27
				      "9s", // 28
				      "9h", // 29
				      "9d", // 30
				      "9c", // 31
				      "Ts", // 32
				      "Th", // 33
				      "Td", // 34
				      "Tc", // 35
				      "Js", // 36
				      "Jh", // 37
				      "Jd", // 38
				      "Jc", // 39
				      "Qs", // 40
				      "Qh", // 41
				      "Qd", // 42
				      "Qc", // 43
				      "Ks", // 44
				      "Kh", // 45
				      "Kd", // 46
				      "Kc", // 47
				      "As", // 48
				      "Ah", // 49
				      "Ad", // 50
				      "Ac"  // 51
};

/* A struct for holding the pieces of the agent.
 * Your program won't have to make use of this struct.
 * Instead, rgbr_test and our RGBR program will use this struct to
 * store pointers to the functions in your .so file.
 */
typedef struct {
  void *(*init_private_info)( char *arg );
  int (*get_action_probs)( const void *private_info, 
			   int round,
			   const int board_cards[ RGBR_MAX_BOARD_CARDS ],
			   const int num_actions[ RGBR_NUM_ROUNDS ],
			   int actions[ RGBR_NUM_ROUNDS ][ RGBR_MAX_ACTIONS_IN_ROUND ],
			   const int num_private_hands,
			   int private_hands[ RGBR_MAX_HANDS ][ RGBR_NUM_HOLE_CARDS ],
			   double probs[ RGBR_MAX_HANDS ][ RGBR_NUM_ACTIONS ] );
  void *dl_obj;
  char *arg_string;
  void *private_info;
} rgbr_agent_t;

#endif
