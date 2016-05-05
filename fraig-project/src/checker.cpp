#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
using namespace std;
string trans( int tt ){
  string ss = "";
  if( tt < 10 ){
    ss = "0";
    while( tt -- ) ss[ 0 ] ++;
    return ss;
  }else{
    ss = "00";
    for( int i = 0 ; i < tt % 10 ; i ++ ) ss[ 1 ] ++;
    for( int i = 0 ; i < tt / 10 ; i ++ ) ss[ 0 ] ++;
    return ss;
  }
  return ss;
}
vector<string> v1 , v2;
int main(){
  int x; cin >> x;
  string myName = "fec" + trans( x ) + ".out";
  string refName = "ref_fec" + trans( x ) + ".out";
  ifstream myIn( myName.c_str() );
  ifstream refIn( refName.c_str() );
  if( !myIn.is_open() )
    cout << "can't open fec#.out!!" << endl;
  if( !refIn.is_open() )
    cout << "can't open ref_fec#.out!!" << endl;
  string ts;
  while( getline( myIn , ts ) ){
    if( ts[ 0 ] == '[' ){
      string tts = "";
      int xx = 0 , ll = ts.length();
      while( ts[ xx ] != ']' ) xx ++;
      for( int i = xx + 1 ; i < ll ; i ++ )
        tts += ts[ i ];
      v1.push_back( tts );
    }
  }
  while( getline( refIn , ts ) ){
    if( ts[ 0 ] == '[' ){
      string tts = "";
      int xx = 0 , ll = ts.length();
      while( ts[ xx ] != ']' ) xx ++;
      for( int i = xx + 1 ; i < ll ; i ++ )
        tts += ts[ i ];
      v2.push_back( tts );
    }
  }
  if( v1.size() != v2.size() ){
    cout << "size not match : ";
    cout << "my(" << v1.size() << ") ";
    cout << "ref(" << v2.size() << ")\n";
    // return 0;
  }
  sort( v1.begin() , v1.end() );
  sort( v2.begin() , v2.end() );
  for( size_t i = 0 ; i < min( v1.size() , v2.size() ) ; i ++ )
    if( v1[ i ] != v2[ i ] ){
      cout << "diff : ";
      cout << "my(" << v1[ i ] << ") ";
      cout << "ref(" << v2[ i ] << ")\n";
      // break;
    }
  cout << "Fec groups are the same" << endl;
}
