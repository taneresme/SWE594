#include <stdio.h>
#include <iostream>
#include <string>
#include <list>
#include <cmath>
#include <omp.h>

int N = 1;
std::list<int> primes;

std::list<int> serialPrimes(int N){
	std::list<int> r(1,2);
	for(int i=3; i<=N; i +=2){
		for(int j : r){
			if(i%j==0) break;
			if(i/j<=j) { r.push_back(i); break; }
		}
	}
	return r;
}

std::list<int> parallelPrimes(int N, std::list<int> checklist){
	std::list<int> a; return a;
}

void formatOutput(){
	
}

void append(std::list<int> l1, std::list<int> l2){
	l1.splice(l1.end(),l2);
}

int main(int argc, char *argv[])
{
	if(argc>2) { std::cout << "Please enter at most 1 additional (integer) argument."; return 1; }
	if(argc==2) N = std::stoi(argv[1]);
	else{
		std::cout << "Enter an integer:\t";
		std::string s; std::cin  >> s; N = std::stoi(s);
	}
	
	primes = serialPrimes((int)sqrt(N));
	
	//for(int i : primes) std::cout << i << '\t';
	//std::cout << std::endl;
	return 0;
}
