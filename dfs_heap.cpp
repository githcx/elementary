#include <iostream>
#include <stack>
using namespace std;

struct Iter
{
  Iter(int num_):num(num_),stat(-1){}
  int num;
  int stat;
};

int main()
{
  int maxNum = 10;

  stack<Iter> path;

  // push root node with num eq 0
  Iter newTop(0);
  path.push(newTop);

  while(!path.empty())
  {
    auto& curTop = path.top();

    ++curTop.stat;

    // return from right or reach max num
    if( curTop.num > maxNum || curTop.stat >= 2 ) { path.pop(); continue; }

    // enter into left
    if( curTop.stat == 0 )
    {
      Iter newTop(curTop.num * 2 + 1);
      path.push(newTop);
      continue;
    }

    // return from left, output num
    cout << curTop.num << endl;

    // enter into right
    Iter newTop(curTop.num * 2 + 2);
    path.push(newTop);
  }
  return 0;
}
