#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <list>
#include <cmath>
#include <omp.h>
#include <time.h>

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
	std::list<int> primes;
	double time;
};

long int N = 1;
double ts = 0;

output findPrimes(int N, int threadCount, omp_sched_t schedule, int chunk){
	output r;
	int _sqrt = (int)sqrt(N);
	int i;
	//SERIAL SECTION
	r.primes = std::list<int>(1,2);
	for(i=3; i<=_sqrt; i+=2){
		for(int j : r.primes){
			if(i%j==0) break;
			if(i/j<=j) { r.primes.push_back(i); break; }
		}
	}
	_sqrt = r.primes.size(); //This variable now stores the number of primes less than sqrt(N).
	
	//PARALLEL SECTION
	std::list<int>::iterator it; int k;
	r.time = omp_get_wtime();
	for(;i<=N; i+=2){
		for(it=r.primes.begin(), k=0; k<_sqrt; k++, it++){
			if(i%(*it)==0) break;
			if(i/(*it)<=(*it)) { r.primes.push_back(i); break; }
		}
	}
	r.time = omp_get_wtime()-r.time;
	
	r.primes.sort();
	return r;
}

std::string formatOutput(omp_sched_t schedule, std::string _schedule, int chunk){
	double t1, t2, t4, t8, t12, s2, s4, s8;
	t1 = findPrimes(N, 1, schedule, chunk).time;
	t2 = findPrimes(N, 2, schedule, chunk).time;
	t4 = findPrimes(N, 4, schedule, chunk).time;
	t8 = findPrimes(N, 8, schedule, chunk).time;
	t12= findPrimes(N,12, schedule, chunk).time;
	s2 = ts/t2;
	s4 = ts/t4;
	s8 = ts/t8;
	std::ostringstream s;
	s <<N<<',' <<_schedule<<',' <<chunk<<',' <<t1<<',' <<t2<<','
		<<t4<<',' <<t8<<',' <<t12<<',' <<s2<<',' <<s4<<',' <<s8<<'\n';
	return s.str();
}

/*void append(std::list<int> l1, std::list<int> l2){
	l1.splice(l1.end(),l2);
}*/

int main(int argc, char *argv[])
{
	//INPUT ACCEPTANCE
	if(argc>2) { std::cout << "Please enter at most 1 additional (integer) argument."; return 1; }
	if(argc==2) N = std::stoi(argv[1]);
	else{
		std::cout << "Enter an integer:\t";
		std::string s; std::cin  >> s; N = std::stoi(s);
	}
	
	//CALCULATING SEQUANTIAL TIME
	std::list<int> P(1,2);
	ts = omp_get_wtime();
	for(int i=3; i<=N; i +=2){
		for(int j : P){
			if(i%j==0) break;
			if(i/j<=j) { P.push_back(i); break; }
		}
	}
	ts = omp_get_wtime()-ts;
	
	
	//OUTPUT FILE CREATION
	std::ofstream ofs;
	ofs.open("results.csv");
	ofs << "M,Scheduling,Chunk,T1,T2,T4,T8,T12,S2,S4,S8\n";
	
	//FINDING PRIMES REPEATEDLY
	ofs << formatOutput(omp_sched_static, "Static", 10);
	
	ofs.close();
	return 0;
}
