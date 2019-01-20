/**************************************************
  SGA - Simple Genetic Algorithm
  ------------------------------
  Sin Bowl minimization 2012
    - recent bug fixes:   09/2012 by John Johnson
    - slight mod to rand: 11/2017 by Scott Gordon
  ------------------------------
  SGA Adapted from David Goldberg:
    "Genetic Algorithms in Search, Optimization, and Machine Learning"
     Addison-Wesley, 1989.
  ------------------------------
  SGA Modifications to handle functions with parameters X and Y
    - Added Two-Point Crossover Operator    Starts at line: 278
    - Added Uniform Crossover Operator      Starts at line: 320
    Craig Hulsebus 12/10/2018
  ------------------------------
  Unix version
  Compile with:  g++ sga.c
***************************************************/

#include <iostream>
#include <cmath>
#include <cstdlib>
#include <time.h>
using namespace std;

#define POPULATION_SIZE 20    // population size - number of strings
#define CHROM_LENGTH    32    // binary string length of each individual
#define PMUT            0.03  // probability of flipping each bit
#define MAX_GEN         100    // GA stops after this many generations
#define GEN_REP         1     // report is generated at these intervals
#define ELITE           1     // 1=elitism,  0=no elitism
#define MAXMIN          -1    // -1=minimize, 1=maximize
#define CROSSOVER       3     // 3=Uniform, 2=TwoPoint, 1=One-Point

/***************************************************************
****  random fraction between 0.0 and 1.0                  *****
****************************************************************/
#define fracrand() ((double)rand()/RAND_MAX)

void   initialize_population();
void   crossover1(int parent1, int parent2, int child1, int child2); // One-Point Crossover
void   crossover2(int parent1, int parent2, int child1, int child2); // Two-Point Crossover
void   crossover3(int parent1, int parent2, int child1, int child2); // Uniform Crossover
void   mutation();
void   tselection();
void   decode(int index);
void   getpreviousbest();
double evaluate(int valueX, int valueY);
double convRange(int raw);
int    flip(double prob);
void   statistics();
void   elite();
void   finalreport();

struct individual
{
  int valueX, valueY;    
  unsigned char string[CHROM_LENGTH];
  double fitness;   
};

struct individual pool[POPULATION_SIZE];        
struct individual new_pool[POPULATION_SIZE];    
struct individual beststring, verybest;         

int selected[POPULATION_SIZE];                  
int generations;                                

/*********************************************************/
int main()
{
  cout.setf(ios::fixed); cout.setf(ios::showpoint); cout.precision(4);
  int i;
  generations = 0;
  if (MAXMIN==-1) verybest.fitness = 999999; else verybest.fitness=-999999;

  srand(time(NULL));
  initialize_population();
  generations = 1;

  do
  {
    getpreviousbest();

    /*** SELECTION ***/        
    tselection();

    /*** CROSSOVER ***/     
    /* One-Point Crossover */
    if (CROSSOVER==1) 
    {
      for (i=0; i<POPULATION_SIZE; i=i+2)           
        crossover1(selected[i],selected[i+1],i,i+1); 
    }
    /* Two-Point Crossover */
    else if (CROSSOVER==2) 
    {
      for (i=0; i<POPULATION_SIZE; i=i+2) 
        crossover2(selected[i],selected[i+1],i,i+1);
    }
    /* Unifrom Crossover */
    else if (CROSSOVER==3) 
    {
      for (i=0; i<POPULATION_SIZE; i=i+2) 
        crossover3(selected[i],selected[i+1],i,i+1);
    }
    /* Redundancy check to make sure a crossover is selected */
    /* One-Point Crossover */
    else 
    {
      for (i=0; i<POPULATION_SIZE; i=i+2) 
        crossover1(selected[i],selected[i+1],i,i+1);
    }                         

    /*** MUTATION ***/
    mutation();                        

    /*** EVALUATE ***/
    for (i=0; i<POPULATION_SIZE; i++)               
    {
      decode(i);      
      pool[i].fitness = evaluate(pool[i].valueX, pool[i].valueY);  
                               
    }                                                

    if (ELITE==1)              
      elite();

    if (generations % GEN_REP == 0)
      statistics();

  } while (++generations < MAX_GEN);

  finalreport();
  return(0);
}

/*********************************************************
  3-2 Tournament Selection                                     
**********************************************************/

void tselection()                                 
{ int i;
  for (i=0; i<POPULATION_SIZE; i+=2)
  {
    int r = (int) (fracrand()*POPULATION_SIZE);
    int s = (int) (fracrand()*POPULATION_SIZE);
    int t = (int) (fracrand()*POPULATION_SIZE);

    if ( ((MAXMIN*pool[r].fitness) >= (MAXMIN*pool[s].fitness))
      || ((MAXMIN*pool[r].fitness) >= (MAXMIN*pool[t].fitness)))
    {
      if ((MAXMIN*pool[s].fitness) > (MAXMIN*pool[t].fitness))
        { selected[i] = r; selected[i+1] = s; }
      else
        { selected[i] = r; selected[i+1] = t; }
    }
    else
    {
      if ( ((MAXMIN*pool[s].fitness) >= (MAXMIN*pool[r].fitness))
        || ((MAXMIN*pool[s].fitness) >= (MAXMIN*pool[t].fitness)))
      {
        if ((MAXMIN*pool[r].fitness) > (MAXMIN*pool[t].fitness))
        { selected[i] = s; selected[i+1] = r; }
        else
        { selected[i] = s; selected[i+1] = t; }
      }
      else
      {
        if ( ((MAXMIN*pool[t].fitness) >= (MAXMIN*pool[r].fitness))
          || ((MAXMIN*pool[t].fitness) >= (MAXMIN*pool[s].fitness)))
        {
          if ((MAXMIN*pool[r].fitness) > (MAXMIN*pool[s].fitness))
          { selected[i] = t; selected[i+1] = r; }
          else
          { selected[i] = t; selected[i+1] = s;}
} } } } }

/*********************************************************
  Elitism - copy best string to 0th position of new pool
**********************************************************/
void elite()
{
  decode(0);
  if ((MAXMIN*beststring.fitness) > (MAXMIN*evaluate(pool[0].valueX, pool[0].valueY)))
  {
    pool[0].fitness = beststring.fitness;
    pool[0].valueX = beststring.valueX;       // valueX at the top position of pool replaced with beststring of valueX.
    pool[0].valueY = beststring.valueY;       // valueY at the top position of pool replaced with beststring of valueY.
    for (int j=0; j<CHROM_LENGTH; j++)                
      pool[0].string[j] = beststring.string[j];
  }
}

/*********************************************************
    Initialize pool to random binary values
**********************************************************/
void initialize_population()
{
  for (int i=0; i<POPULATION_SIZE; i++)
  {
    for (int j=0; j<CHROM_LENGTH; j++ )
      pool[i].string[j] = flip(0.5);
    decode(0);                              // Pass 0 into decode. Decode then handles putting valueX and valueY into the pool .
    pool[i].fitness = evaluate(pool[i].valueX, pool[i].valueY);    // Evaluate pool positions of valueX and valueY to fetermine fitness.
  }
  statistics();
}

/*************************************************************
  - Determine and display best string from previous generation.
  - Maintain very best string from all runs.
**************************************************************/
void getpreviousbest()
{
  cout.setf(ios::fixed); cout.setf(ios::showpoint); cout.precision(4);

  if (MAXMIN==-1) beststring.fitness=999999; else beststring.fitness=-999999;

  for (int i=0; i<POPULATION_SIZE; i++)
  {
    if ((MAXMIN*pool[i].fitness) > (MAXMIN*beststring.fitness))
    {
      beststring.valueX = pool[i].valueX;         // Beststring of valueX 
      beststring.valueY = pool[i].valueY;         // Beststring of valueY
      for (int j=0; j<CHROM_LENGTH; j++)
        beststring.string[j] = pool[i].string[j];
      beststring.fitness = pool[i].fitness;
    }
  }   

  if (generations % GEN_REP == 0)
  {
    cout << endl << "   Best string: ";
    for (int j=0;j<CHROM_LENGTH;j++)
      cout << (int) beststring.string[j];
    cout << " valueX: " << convRange(beststring.valueX);      // Print the bestring for valueX
    cout << " valueY: " << convRange(beststring.valueY);      // Print the bestring for valueY
    cout << " fitness: " << beststring.fitness << endl;       // Print the fitness for valueX and valueY.
  }

  if ((MAXMIN*beststring.fitness) > (MAXMIN*verybest.fitness))
  {
    verybest.valueX = beststring.valueX;        // Storing the beststring into verybest for valueX
    verybest.valueY = beststring.valueY;        // Storing the beststring into verybest for valueY
    for (int j=0; j<CHROM_LENGTH; j++)
      verybest.string[j] = beststring.string[j];
    verybest.fitness = beststring.fitness;
  }
}

/*********************************************************
      one-point crossover            
**********************************************************/
void crossover1(int parent1, int parent2, int child1, int child2)
{
  int i, site;
  site = (int) (fracrand()*CHROM_LENGTH);
  for (i=0; i<CHROM_LENGTH; i++)
  {
    if ((i<=site) || (site==0))
    {
      new_pool[child1].string[i] = pool[parent1].string[i];
      new_pool[child2].string[i] = pool[parent2].string[i];
    }
    else
    {
      new_pool[child1].string[i] = pool[parent2].string[i];
      new_pool[child2].string[i] = pool[parent1].string[i];
} } }

/*********************************************************
      Two-Point Crossover           
**********************************************************/
void crossover2(int parent1, int parent2, int child1, int child2)
{
  int i, site, site2;
  site = (int) (fracrand()*CHROM_LENGTH);           // Random crossover segmant point.
  site2 = (int) (fracrand()*CHROM_LENGTH);          // Random crossover segmant point.
  for (i=0; i<CHROM_LENGTH; i++)
  {
    if (site < site2) {              // If the site random segmant is smaller than the site2 random segmant.
      if ((i<=site) || (site==0))
      {
        new_pool[child1].string[i] = pool[parent1].string[i];       // Parent1 to child1
        new_pool[child2].string[i] = pool[parent2].string[i];       // Parent2 to child2
      }
      else if ((i > site) && (i < site2))       // Handle the bits inbetween the two segmants. 
      {
        new_pool[child1].string[i] = pool[parent2].string[i];       // Parent2 to child1
        new_pool[child2].string[i] = pool[parent1].string[i];       // Parent1 to child2
      }
      else {                                    // Handle bits when i > site2 segment.
        new_pool[child1].string[i] = pool[parent1].string[i];       // Parent1 to child1
        new_pool[child2].string[i] = pool[parent2].string[i];       // Parent2 to child2
      }
    }
    else {                           // If the site2 random segmant is smaller than the site random segmant.
      if ((i<=site2) || (site==0))        
        {
        new_pool[child1].string[i] = pool[parent1].string[i];       // Parent1 to child1
        new_pool[child2].string[i] = pool[parent2].string[i];       // Parent2 to chuld2
      }
      else if ((i > site2) && (i < site))       // Handle the bits inbetween the two segmants. 
      {
        new_pool[child1].string[i] = pool[parent2].string[i];       // Parent2 to child1
        new_pool[child2].string[i] = pool[parent1].string[i];       // Parent1 to child2
      } 
      else {                                    // Handle bits when i > site segment.
        new_pool[child1].string[i] = pool[parent1].string[i];       // Parent1 to child1
        new_pool[child2].string[i] = pool[parent2].string[i];       // Parent2 to child2
} } } }
    
/*********************************************************
      Uniform Crossover           
**********************************************************/
void crossover3(int parent1, int parent2, int child1, int child2)
{
  int i, crossProb;
  srand(time(NULL));             

  for (i=0; i<CHROM_LENGTH; i++)
  {
    crossProb = rand() % 2;                    // Randomly chooses between 0 or 1.  
    if (crossProb == 0) {                                       // If croosProb is 0, then parent1 1-bit goes to child1
      new_pool[child1].string[i] = pool[parent1].string[i];                             // parent2 1-bit goes to child2
      new_pool[child2].string[i] = pool[parent2].string[i];
    }
    else {                                                      // If croosProb is 1, then parent2 1-bit goes to child1
      new_pool[child1].string[i] = pool[parent2].string[i];                             // parent1 1-bit goes to child2
      new_pool[child2].string[i] = pool[parent1].string[i];
} } }   

/*********************************************************
    Bitwise mutation  - also transfers strings to pool
**********************************************************/
void mutation()
{
  int i,j;
  for (i=0; i<POPULATION_SIZE; i++)
  {
    for (j=0; j<CHROM_LENGTH; j++)
      if (flip(PMUT)==1)
        pool[i].string[j] = ~new_pool[i].string[j] & 0x01;
      else
        pool[i].string[j] = new_pool[i].string[j] & 0x01;
  }
}

/*********************************************************
    Convert bitstring to positive integer     
**********************************************************/
void decode(int index)        // Switched to void function so valueX and valueY can be directly changed when called. 
{
  pool[index].valueX = 0;           // Initialize valuex in pool[index] to 0
  pool[index].valueY = 0;           // Initialize valueY in pool[index] to 0
  for (int i=0; i<16; i++) {
    pool[index].valueX += (int) pow(2.0,(double)i) * pool[index].string[15-i];                // Takes first 16-bits from string and puts them in pool[].valueX
    pool[index].valueY += (int) pow(2.0,(double)i) * pool[index].string[CHROM_LENGTH-1-i];    // Takes last 16-bits from string and puts them in pool[].valueY
  }                           
}

/*********************************************************
   F(X,Y) = 0.75*cos(X) - 0.25*sin(Y) - 0.1*(X) - 0.1*(Y)
*********************************************************/
double evaluate(int valueX, int valueY)
{
  double d1 = convRange(valueX);
  double d2 = convRange(valueY);
  double g = (double) (0.75 * cos(d1) - 0.25 * sin(d2) - .1 * d1 - .1 * d2);
  return(g);
}

/*********************************************************
 Convert positive integer to desired floating point range.
 Problem-specific - change for different string lengths
**********************************************************/
double convRange(int raw)             
{
  double outval = ((((double)raw)/65535.0)*10.0)-5.0;    // Converts the 16-bit number into a range from -5 to +5
  return(outval);                                    
}

/*********************************************************
    Do a biased coin toss based on a probability    
**********************************************************/
int flip(double prob)
{
  return((fracrand()<prob)?1:0);
}

/*********************************************************
    Report printed at each generation
**********************************************************/
void statistics()
{
  int i,j;
  cout.setf(ios::fixed); cout.setf(ios::showpoint); cout.precision(4);

  cout << "\nGENERATION: " << generations << endl << "Selected Strings: ";
  for (i=0; i<POPULATION_SIZE; i++) cout << selected[i] << " ";
  cout << endl << "\n\tX\tY\tf(X,Y)\t\tnew_str\t\t\t\tX\tY";
  for (i=0; i<POPULATION_SIZE; i++)
  {
    cout << endl << "   ";
    cout << convRange(pool[i].valueX)<<"\t"<<convRange(pool[i].valueY)<<"\t"<<pool[i].fitness <<"\t";  // Prints pool[i].valueX, pool[i].valueY and pool[i].fitness
    for (j=0; j<CHROM_LENGTH; j++)
      cout << (int) pool[i].string[j];                // Print the 32-bit string out
    decode(i);
    cout << "\t" << convRange(pool[i].valueX);        // Print pool[i].valueX
    cout << "\t" << convRange(pool[i].valueY);        // Print pool[i].valueY
  }
  cout << endl;
}

/*********************************************************
    Report printed at the very end of execution
**********************************************************/
void finalreport()
{
  cout << "=======================================================" << endl;
  cout << "Best result over all generations:" << endl;
  for (int j=0; j<CHROM_LENGTH; j++)
    cout << (int) verybest.string[j];                 // Print out the verybest string found.
  cout << endl; 
  cout << "Decoded valueX = " << convRange(verybest.valueX);         // Print the verybest valueX found.
  cout << "  Decoded valueY = " << convRange(verybest.valueY);       // Print the verybest valueY found.
  cout << "  Fitness = " << verybest.fitness << endl;                // Print the veryvest fitness found.
}
