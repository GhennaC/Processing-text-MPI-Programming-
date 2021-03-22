#include <cstdlib>
#include <iomanip> 
#include <iostream> 
#include <fstream> 
#include <mpi.h>
#include <sstream>
#include <vector>
#include<bits/stdc++.h> 
#define MASTER 0
#define NUM_THREADS 4
using namespace std;

static string fileIn; // used for reading and writing from/in file
static int numberOfParagraphs = 0; // used for printing in order
void insertConsonant(string &paragraph) {
	for(int i = 6 ; i < paragraph.length(); i++){
		if(strchr("bcdfghjklmnpqrstvwxyzBCDFGHJKLMNPQRSTVWXYZ",paragraph[i]) != NULL){
			if(paragraph[i] < 97)
				paragraph.insert(i+1,1,paragraph[i]+32);
			else  
				paragraph.insert(i+1,1,paragraph[i]);
			
			i++;
		}
	}
}
void upperEvenPosition(string &paragraph) {
	int nr = 0;
	for(int i = 7; i < paragraph.length() ; i++) {
		if(paragraph[i] == 32 || strchr("\n",paragraph[i]))
			nr = 0;
		else{
			nr++;	
			if(nr % 2 == 0){
				if(paragraph[i] >= 97 && paragraph[i] <= 122)
					paragraph[i] -= 32;
			}
		}
		
	}
}
void upperFirstLetter(string &paragraph) {
	for(int i = 8; i < paragraph.length() ; i++){
		if(strchr(",.\n'!? ",paragraph[i-1]) && paragraph[i] <= 122 && paragraph[i] >= 97)
			paragraph[i] -= 32;
	}
}
void reverseWord(string &text ,int k , int l) {
	for(int j = k, x = l ; j < x;j++,x--) {
		char temp = text[j];
		text[j] = text[x];
		text[x] = temp;
	}
}
void reverseEverySeventhWord(string &paragraph) {
	int words = 0; int nrCharacters = 0;
	for(int i = 0 ; i < paragraph.size(); i++) {
		if(strchr("\n",paragraph[i]))
			words = 0;
		if(strchr(" ",paragraph[i])){
			words++;
			if(words % 7 == 0){
				reverseWord(paragraph,i-nrCharacters,i-1);
				words = 0;
			}
			nrCharacters = 0;

		}else nrCharacters++;
	}
}

//function used in multi-threading.
void *reader(void *arg){
	int thread_id = *(int *)arg;
	string line;
	string paragraph;
	paragraph.clear();
	string typeOfParagraph;
	ifstream MyReadFile(fileIn);
	int order = 0;
	// if line empty compare thread id and typeofparagraph for sending
	//else if paragraph add to typeofparagraph,else simple adding to paragraph
	while(getline(MyReadFile, line)) {
		if(line.compare("")){
			if(paragraph.empty()){
				order++;
				typeOfParagraph = line;
				paragraph = paragraph + typeOfParagraph + "\n";
			}else {
				paragraph = paragraph+line+" \n";
			}
		}else {
			if(!typeOfParagraph.compare("horror") && thread_id == 0){
				MPI_Send(paragraph.c_str(), paragraph.size()+1, MPI_CHAR, 1, order, MPI_COMM_WORLD);
			}else if(!typeOfParagraph.compare("comedy") && thread_id == 1){
					MPI_Send(paragraph.c_str(), paragraph.size()+1, MPI_CHAR, 2, order, MPI_COMM_WORLD);
				  }else if(!typeOfParagraph.compare("fantasy") && thread_id == 2){
							MPI_Send(paragraph.c_str(), paragraph.size()+1, MPI_CHAR, 3, order, MPI_COMM_WORLD);
						}else if(!typeOfParagraph.compare("science-fiction") && thread_id == 3){
								MPI_Send(paragraph.c_str(), paragraph.size()+1, MPI_CHAR, 4, order, MPI_COMM_WORLD);
							}
			paragraph.clear();
			typeOfParagraph.clear();
		}
	}

	if(!typeOfParagraph.compare("horror") && thread_id == 0){
		MPI_Send(paragraph.c_str(), paragraph.size()+1, MPI_CHAR, 1, order, MPI_COMM_WORLD);
			}else if(!typeOfParagraph.compare("comedy") && thread_id == 1){
					MPI_Send(paragraph.c_str(), paragraph.size()+1, MPI_CHAR, 2, order, MPI_COMM_WORLD);
				  }else if(!typeOfParagraph.compare("fantasy") && thread_id == 2){
							MPI_Send(paragraph.c_str(), paragraph.size()+1, MPI_CHAR, 3, order, MPI_COMM_WORLD);
						}else if(!typeOfParagraph.compare("science-fiction") && thread_id == 3){
								MPI_Send(paragraph.c_str(), paragraph.size()+1, MPI_CHAR, 4, order, MPI_COMM_WORLD);
							}
	paragraph.clear();
	typeOfParagraph.clear();
	paragraph = "";
	numberOfParagraphs = order;
	MPI_Send(paragraph.c_str(), paragraph.size()+1, MPI_CHAR, thread_id+1, 0, MPI_COMM_WORLD);
	MyReadFile.close();
	pthread_exit(NULL);

}

int main(int argc, char *argv[]) {
	fileIn = argv[1];
 	void *status;
	int rank;
	int nProcesses;
	int provided;
	MPI_Init_thread(&argc, &argv,MPI_THREAD_MULTIPLE, &provided);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);

	if (rank == MASTER) {
		int r; 
		MPI_Status statuss;
		pthread_t threads[NUM_THREADS];
		int arguments[NUM_THREADS];
		char nameOut[1000];
		
		for (int i = 0; i < NUM_THREADS; i++) {
			arguments[i] = i;
			r = pthread_create(&threads[i], NULL, reader, &arguments[i]);
			if (r) {
				printf("Eroare la crearea thread-ului %d\n", i);
				exit(-1);
			}
		}

		for (int i = 0; i < NUM_THREADS; i++) {
			r = pthread_join(threads[i], &status);
			if (r) {
				printf("Eroare la asteptarea thread-ului %d\n", i);
				exit(-1);
			}
		}
		int number = 0;
		fileIn.resize(fileIn.size()-3);
		ofstream MyFile(fileIn+"out");
		vector <string> buffer;
		while(numberOfParagraphs != number){
			number++;
			int len;
			MPI_Status statuss;
			MPI_Probe(MPI_ANY_SOURCE,number,MPI_COMM_WORLD,&statuss);
			MPI_Get_count(&statuss,MPI_CHAR,&len);
			char *paragraph = new char[len];
			MPI_Recv(paragraph,len,MPI_CHAR,MPI_ANY_SOURCE,number,MPI_COMM_WORLD,&statuss);
			string str = paragraph;
			buffer.push_back(str);
		}
		for(string s : buffer){
			MyFile << s <<"\n";

		}
		MyFile.close();
	}else {
		//Workers every rank for an type of paragraph.
		vector<pair <string,int>> buffer;
		while(1){
				int len,tag;
				MPI_Status status;
				MPI_Probe(0,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
				MPI_Get_count(&status,MPI_CHAR,&len);
				char *paragraph = new char[len];
				MPI_Recv(paragraph, len, MPI_CHAR, 0, MPI_ANY_TAG,MPI_COMM_WORLD,&status);
				if(status.MPI_TAG == 0){
					for(pair <string,int> temp : buffer){
						MPI_Send(temp.first.c_str(),temp.first.size()+1, MPI_CHAR, 0, temp.second, MPI_COMM_WORLD);
					}
					break;
				}else {
					
					string temp = paragraph;
					if(rank == 1){
						insertConsonant(temp);
					}
					if(rank == 2) {
						upperEvenPosition(temp);
					}
					if(rank == 3) {
						upperFirstLetter(temp);
					}
					if(rank == 4) {
						reverseEverySeventhWord(temp);
					}
					buffer.push_back(make_pair(temp,status.MPI_TAG));
					delete []paragraph;
				}
				
				
		}
	}
	MPI_Finalize();

}
