#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <omp.h>
#include <time.h>
#include <algorithm>

/* HOW TO USE AND WHY:
 * So, what is going on here? First, note that we're trying to find prime numbers up to N.
 * This means that our "output" will be not N but around ln(N) numbers. We will find those
 * by comparing each (odd) number to previously found primes. "But wait a second! Then this
 * has to be a _sequential_ process, doesn't it?" Well, yes, but we know that any composite
 * number K has a factor up to sqrt(K), and since _every_ number in consideration is bound
 * by N, we know that checking primes up to sqrt(N) will suffice. So idea is: sequentially
 * construct primes up to sqrt(N), then check greater numbers in parallel with those.
 * 
 * This comes with some restrictions. First, these methods lose a lot when attempted to
 * generalize. For example, we cannot start from scratch and determine primes between 2
 * numbers. Second, we should not compare against an _unordered_ list. Third, our final
 * list possibly is hard to manage, that is, of course, if we desire an ordered list. One
 * possible solution is that we can get sublists from each parallel subroutine and combine
 * them as desired, but this necessitates synchronization. Or we can get lazy and order
 * the list after the fact. Also, different parallelization methods can yield unwieldly
 * sublists.
 * 
 * Therefore our expectation from the user is exactly as the project describes: provide
 * the program with a number and be done with it. No generalizations, no further use, a very
 * simple and honest transaction.
 */

struct output{
	std::vector<long int> primes[13];
	double time;
};

struct scheduleDetail{
	omp_sched_t type;
	std::string name;
	
	scheduleDetail(char abbr){
		switch(toupper(abbr)){
			case 'S': name="Static" ; type=omp_sched_static;  break;
			case 'D': name="Dynamic"; type=omp_sched_dynamic; break;
			case 'G': name="Guided" ; type=omp_sched_guided;  break;
			default : std::cout << "Invalid character!\n";	  break;
		}
	}
};

long int N = 1;
//double ts = 0; //To store sequential time, now omitted.
short ind[5]={1,2,4,8,12};

output findPrimes(long int N, int threadCount, omp_sched_t schedule, int chunk){
	output r;
	int _sqrt = (int)sqrt(N);
	int i;
	//SERIAL SECTION
	r.time = omp_get_wtime();
	r.primes[12].push_back(2);
	for(i=3; i<=_sqrt; i+=2){
		for(int j : r.primes[12]){
			if(i%j==0) break;
			if(i/j<j){ r.primes[12].push_back(i); break; }
		}
	}
	_sqrt = r.primes[12].size(); //This variable now stores the number of primes less than sqrt(N).
		//Actually we could now omit this and use a simple for(:) format but
		//this allows us to handle edge case (k==_sqrt) better.
		
	//PARALLEL SECTION
	omp_set_schedule(schedule, chunk);
	int k;
#pragma omp parallel firstprivate(i) shared(r) private (k) num_threads(threadCount)
{
	int tid = omp_get_thread_num();
	#pragma omp for
	for(long int _i=i;_i<=N; _i+=2){
		for(k=0; k<=_sqrt; k++){
			if( (k==_sqrt) || ((_i/r.primes[12][k]) < r.primes[12][k]) )
				{ r.primes[tid].push_back(_i); break; }
			if(_i%(r.primes[12][k])==0) break;
		}
	}
}
	r.time = omp_get_wtime()-r.time;
	
	//MERGING THREAD DATA
	int y=0, z=0;
	for(y; y<threadCount; y++){ z += r.primes[y].size(); }
	r.primes[12].reserve(z+r.primes[12].size());
	for(y; y>=0; ){
		y--;
		r.primes[12].insert( r.primes[12].end(), r.primes[y].begin(), r.primes[y].end() ); 
		r.primes[y].clear();
	}
	std::sort(r.primes[12].begin(), r.primes[12].end());
	
	return r;
}

std::string formatOutput(scheduleDetail schedule, int chunk){
	std::ostringstream st;
	st <<N<<',' <<schedule.name<<',' <<chunk<<',';
	double t[5], s[3];
	int k;
	for(k=0; k<5; k++){
		output p = findPrimes(N, ind[k], schedule.type, chunk);
		t[k] = p.time; st<<t[k]<<',';
		//for(auto i:p.primes) std::cout << i << '\t'; std::cout<<std::endl; //UNCOMMENT TO PRINT PRIMES
		}
	for(k=0; k<3; k++){ s[k]=t[0]/t[k+1]; st<<s[k]; if(k<2) st<<','; }
	st<<'\n';
	return st.str();
}

int main(int argc, char *argv[])
{
	//Letting compiler know that we'll be working in parallel
	omp_set_schedule(omp_sched_dynamic, 0);
	
	//INPUT ACCEPTANCE
	if(argc>2) { std::cout << "Please enter at most 1 additional (integer) argument."; return 1; }
	if(argc==2) N = std::stoi(argv[1]);
	else{
		std::cout << "Enter an integer:\t";
		std::string s; std::cin  >> s; N = std::stoi(s);
	}
		
/*	//CALCULATING SEQUANTIAL TIME //omitted since we're taking t0 instead of t*
	std::list<int> P(1,2);
	ts = omp_get_wtime();
	for(int i=3; i<=N; i +=2){
		for(int j : P){
			if(i%j==0) break;
			if(i/j<=j) { P.push_back(i); break; }
		}
	}
	ts = omp_get_wtime()-ts;*/
	
	
	//OUTPUT FILE CREATION
	std::ofstream ofs;
	ofs.open("results.csv");
	ofs << "M,Scheduling,Chunk,T1,T2,T4,T8,T12,S2,S4,S8\n";
	
	//FINDING PRIMES REPEATEDLY AND RECORDING THE RESULTS
	ofs << formatOutput(scheduleDetail('S'), 50);
	ofs << formatOutput(scheduleDetail('S'),100);
	ofs << formatOutput(scheduleDetail('D'), 50);
	ofs << formatOutput(scheduleDetail('D'),100);
	ofs << formatOutput(scheduleDetail('G'), 50);
	ofs << formatOutput(scheduleDetail('G'),100);
	
	ofs.close();
	return 0;
}
