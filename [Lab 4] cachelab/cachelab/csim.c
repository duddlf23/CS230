// 20150756 Choi Young-Il


#include "cachelab.h"
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
struct aa{
    char valid;
    unsigned long long tag;
    int num;
};
struct aa*** cache;
int s,E,b;
unsigned long long address;
int hit=0, miss=0, eviction=0;
int main(int argc, char **argv)
{
    unsigned long long S, tag, i, m;
    int j,x,cnt,cnt2,cnt3,minn,minj;
    char c;
    char* file_name;
    while((c=getopt(argc, argv, "s:E:b:t:v")) != -1){
	if(c=='s')
	    s = atoi(optarg);
	else if (c=='E')
	    E = atoi(optarg);
	else if (c=='b')
	    b = atoi(optarg);
	else if (c=='t')
	    file_name = optarg;	
    }
    S = 1 << s;
    cache=(struct aa***) malloc(S * sizeof(struct aa**));
    for(i=0; i<S; i++){
	cache[i] = (struct aa**) malloc(E * sizeof(struct aa*));
	for(j=0; j<E; j++){
	    cache[i][j] = (struct aa*) malloc(sizeof(struct aa));
	    cache[i][j] -> valid = 0;
	}
    }
    cnt3=0;
    m = (1 << (s + b)) - 1;
    FILE *in = fopen(file_name, "r");
    while (fscanf(in, " %c %llx,%d", &c, &address, &x)==3){
        cnt3++;
	if(c=='I') continue;
	if(c!='S' && c!='L' && c!='M') continue;
	tag = address >> (s+b);
	i = (address & m) >> b;
	cnt=1,cnt2=0;
	//printf("%c %llx %lld %lld\n",c,address,tag,i);
	for(j=0; j<E; j++){
	    if (cache[i][j] -> valid==1){
		cnt2 = j+1;
		if(cache[i][j] -> tag == tag){
		    cnt=0;
		    cache[i][j] -> num = cnt3;
		    hit++;
		    break;
		}
	    }
	}
	if (cnt==1){
	    miss++;
	    if(cnt2==E){
	    	minn=cache[i][0] -> num;
		minj=0;
		for(j=1; j<E; j++){
		    if(cache[i][j] -> num < minn){
			minn = cache[i][j] -> num;
			minj = j;
		    }
		}
		cache[i][minj] -> tag = tag;
		cache[i][minj] -> num = cnt3;
		eviction++;
	    }else{
		cache[i][cnt2] -> valid = 1;
		cache[i][cnt2] -> tag = tag;
		cache[i][cnt2] -> num = cnt3;
	    }
	}
	if(c=='M') hit++;
    }
    for(i=0; i<S; i++){
	for(j=0; j<E; j++){
	    free(cache[i][j]);
	}
	free(cache[i]);
    }
    free(cache);
    printSummary(hit, miss, eviction);
    return 0;
}
