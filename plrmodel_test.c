#include "plrmodel.h"




int main()
{
  struct plrmodel_node *n1, *n2;
  n1 = (struct plrmodel_node *)generate_empty_plrmodel_tree(NULL, NULL, 0, 3, 3);
  n2 = (struct plrmodel_node *)recursive_copy_node((void*)n1, NULL);
  divide_slot((void*)n1, 0);
  divide_slot((void*)n1, 0);
  divide_slot((void*)n1, 0);
  divide_slot((void*)n1, 0);
  divide_slot((void*)n1, 0);
  divide_slot((void*)n1, 0);
  divide_slot((void*)n1, 0);
  divide_slot((void*)n1, 0);
  divide_slot((void*)n1, 0);
  recursive_delete_node((void *)n1);
  recursive_delete_node((void *)n2);


}
