#include <cstdlib>
#include <algorithm>
#include <cassert>
#include <vector>
#include <iostream>
using namespace std;

struct Tree
{
  Tree* left;
  Tree* right;
  Tree()
  {
    left = NULL;
    right= NULL;
  }
};

Tree* MakeTree(int n)
{
  Tree* root = new Tree[n];
  for(int i = 0; i < n; i++)
  {
    if( 2*i + 1 < n ) root[i].left = &root[2*i+1];
    if( 2*i + 2 < n ) root[i].right= &root[2*i+2];
  }
  return root;
}

int CA(Tree* root, Tree* p, Tree* q, int cur, Tree*& res)
{
  res = NULL;
  if( !root || !p || !q ) return cur;

  int orig = cur;
  if( ( root == p || root == q ) && ++cur == 2 ) return cur;
  if ( cur == 1 && p == q ) return cur;

  Tree* children[] = { root->left, root->right };
  for(auto& child : children)
  {
    Tree* local = NULL;
    cur = CA(child, p, q, cur, local);

    if ( cur - orig == 2 )
    {
      if( local ) res = local;
      else res = root;
      return cur;
    }
  }

  return cur;
}

Tree* LowestCommonAncestor(Tree* root, Tree* p, Tree* q)
{
  Tree* res = NULL;
  int n = CA(root, p, q, 0, res);

  if( !res )
    if( p == q && n == 1 )
      return p;

  return res;

}

Tree* RandNode(Tree* root, int n)
{
  int rand_idx = rand() % n;
  return &root[rand_idx];
}

vector<int> ancestors(Tree* node, Tree* root)
{
  int index = node - root;
  vector<int> anc;
  for (;;)
  {
    anc.push_back(index);
    if(--index < 0 ) break;
    index /= 2;
  }
  sort(anc.begin(), anc.end());
  return anc;
}

Tree* CommonBrutal(Tree* root, Tree* p, Tree* q)
{
  if(p == q) return p;

  auto panc = ancestors(p, root);
  auto qanc = ancestors(q, root);
  int min = panc.size() > qanc.size() ? qanc.size() : panc.size();
  for(int i = min - 1; i >= 0; i--)
  {
    if (panc[i] == qanc[i]) return root + panc[i];
  }
  return root;
}

int main()
{
  int n = 100;
  Tree* root = MakeTree(n);

  // 1. test 1000 rand pairs, q & q
  for(int i = 0; i < 10000; i++)
  {

    Tree* p = RandNode(root, n);
    Tree* q = RandNode(root, n);
    // cout << "p:" << p - root << endl;
    // cout << "q:" << q - root << endl;

    Tree* standard = CommonBrutal(root, p, q);
    Tree* result = LowestCommonAncestor(root, p, q);
    // cout << "standard:" << standard - root << endl;
    // cout << "result:" << result - root << endl;

    assert(standard == result);

  }

  // 2. test every node, p = q = node
  for (int i = 0; i < n; ++i)
  {
    Tree* p, *q;
    p = q = root + i;

    Tree* standard = CommonBrutal(root, p, q);
    Tree* result = LowestCommonAncestor(root, p, q);

    assert(standard == result);
  }
}
