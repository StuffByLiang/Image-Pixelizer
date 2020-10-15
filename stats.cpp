
#include "stats.h"

stats::stats(PNG & im){

/*
  we need two loops to loop through both x and y

  we need a way to keep track of the sum so far as we loop
    > a way is to just check the previous sum value inserted into the arrays

  we're gonna have to do this for both red, green and blue, we can probably
  reuse the same loop for both sum and sumsq

  use sentinal values when x or y = 0, fill with 0

  sum at (x, y) = sum at (x-1, y) + (x, y-1) + rgb value at (x, y)
    > for sq, we would just square the rgb value at (x, y)

  we can assume that as we loop through x and y, everything above and everything
  to the left will be calculated.

  loops thru x
    loops thru y
      initialize prevSum = 0
        if (x > 0) prevSum += sum(x-1, y)
        if (y > 0) prevSum += sum(x, y-1)
        get rgb value at (x, y)
        store prevSum + rgbValue in sum(x, y)
        store prevSum + rgbValue^2 in sumsq(x, y)

*/
  unsigned width  = im.width();
  unsigned height = im.height();

  vector< vector< vector< long >>* > fields = {
    &sumRed,
    &sumGreen,
    &sumBlue,
    &sumsqRed,
    &sumsqGreen,
    &sumsqBlue
  };

  vector< long > sum = {
    0, // prevSumRed
    0, // prevSumGreen
    0, // prevSumBlue
    0, // prevSumsqRed
    0, // prevSumsqGreen
    0  // prevSumsqBlue
  };

  // initilaize all fields with the known size
  for (vector< vector< long >>* field : fields) {
    (*field).resize(width + 1, vector<long>(height + 1, 0)); 
  }

  for(unsigned x = 1; x <= width; x++) {
    for(unsigned y = 1; y <= height; y++) {
      std::fill(sum.begin(), sum.end(), 0); // reset these values
      
      for (unsigned i = 0; i < fields.size(); i++) {
        sum[i] = (*fields[i])[x - 1][y] + (*fields[i])[x][y - 1] - (*fields[i])[x - 1][y - 1];
      }

      RGBAPixel* pixel = im.getPixel(x-1, y-1);
      long rgbValue[3] = {
        pixel->r, // rgbValue[0] = red
        pixel->g, // rgbValue[1] = green
        pixel->b, // rgbValue[2] = blue
      };

      for(int i=0; i<3; i++) {
        sum[i]     += rgbValue[i]; // add to sum
        sum[i + 3] += rgbValue[i] * rgbValue[i]; // add to sumsq
      }

      for(int i=0; i<6; i++) {
        (*fields[i])[x][y] = sum[i]; // update class fields
      }
    }
  }

}

long stats::calculateSum(vector< vector< long >> & sum, int x, int y, long sideLength) {
  return sum[x+sideLength][y+sideLength]
       - sum[x+sideLength][y]
       - sum[x][y+sideLength]
       + sum[x][y];
}

long stats::getSum(char channel, pair<int,int> ul, int dim){
  long side = getSideLength(dim);
  int x = ul.first;
  int y = ul.second;

  if (channel == 'r') return calculateSum(sumRed, x, y, side);
  if (channel == 'b') return calculateSum(sumBlue, x, y, side);
  else return calculateSum(sumGreen, x, y, side);
}

long stats::getSumSq(char channel, pair<int,int> ul, int dim){
  long side = getSideLength(dim);
  int x = ul.first;
  int y = ul.second;

  if (channel == 'r') return calculateSum(sumsqRed, x, y, side);
  if (channel == 'b') return calculateSum(sumsqBlue, x, y, side);
  else return calculateSum(sumsqGreen, x, y, side);
}

long stats::rectArea(int dim) {
  return pow( getSideLength(dim), 2);
}

long stats::getSideLength(int dim) {
  return pow(2, dim);
}

// given a rectangle, compute its sum of squared deviations from mean, over all color channels.
// see written specification for a description of this function.
double stats::getVar(pair<int,int> ul, int dim){
  long area = rectArea(dim);
  double rVar = getSumSq('r', ul, dim) - pow(getSum('r', ul, dim), 2) / area;
  double gVar = getSumSq('g', ul, dim) - pow(getSum('g', ul, dim), 2) / area;
  double bVar = getSumSq('b', ul, dim) - pow(getSum('b', ul, dim), 2) / area;

  return rVar + gVar + bVar;
}
		
RGBAPixel stats::getAvg(pair<int,int> ul, int dim){
  int area = rectArea(dim);
  int r = getSum('r', ul, dim) / area;
  int g = getSum('g', ul, dim) / area;
  int b = getSum('b', ul, dim) / area;
  return RGBAPixel(r, g, b);
}
