#include <iostream>
#include <vector>
#include <string>
#include <time.h>

using namespace std;

#define NUMBER_OF_REQUESTS 100

int maxCoins = 1000000;

enum NodeType{
    HONEST,
    MALICIOUS    
};

enum NewsType{
    TRUE,
    FAKE
};

class Node{
    public:
    Node(NodeType node_type, double balance, double credibility, double prob, vector <string> expertise){
        this->node_type = node_type;
        this->balance = balance;
        this->credibility = credibility;
        this->prob = prob;
        this->expertise = expertise;
    }

    NodeType node_type;
    double balance;
    double credibility;
    double prob;
    vector <string> expertise;
};

class News{
    public:
    News(NewsType news_type, string category, double amount){
        this->news_type = news_type;
        this->news_category = category;
        this->amount = amount;
    }

    NewsType news_type;
    string news_category;
    double amount;
};

int randomNumberGenerator(int lower, int upper){
//    srand(time(0));
    return lower + rand()%(upper-lower+1);
}

bool resultUsingProb(double prob, bool res){
    return ((double)rand() / RAND_MAX) < prob ? res : !res;
}

int main(int argc, char* argv[]){
    vector<Node *> node;
    int n, p, q;

    n = atoi(argv[1]);
    q = atoi(argv[2]);
    p = atoi(argv[3]);

    for(int i=0; i<n; i++){
        Node* fact_checker;
        if(i < (1-q)*n){
            if(i < p*n){
                fact_checker = new Node(HONEST, 100, 50, 0.9, {"fd"});
            }
            else{
                fact_checker = new Node(HONEST, 100, 50, 0.7, {"fd"});
            }
        }
        else{
            fact_checker = new Node(MALICIOUS, 100, 50, 0, {"fd"});
        }
        node.push_back(fact_checker);
    }

    for(int t=0; t<NUMBER_OF_REQUESTS; t++){
        // generating request
        int check_requester = randomNumberGenerator(0, n-1);
        int count = 101;
        
        while(node[check_requester]->balance < 10 && count--)	check_requester = randomNumberGenerator(0, n-1);

        News* news_to_be_checked = new News(resultUsingProb(0.5, true) ? TRUE : FAKE, "dfsd", 10);
        node[check_requester]->balance -= 10;

        // voting and updating their trustworthiness the fact checkers
        double reward = news_to_be_checked->amount / (n-1);
        double sum_credibility = 0;
        double res_voters = 0;
        vector<int> correct_voters;

        for(int i=0; i<n; i++){
            if(i == check_requester)    continue;

            Node* fact_checker = node[i];
            NewsType res = resultUsingProb(fact_checker->prob, news_to_be_checked->news_type == TRUE) ? TRUE : FAKE;
//            cout<<(bool)(news_to_be_checked->news_type == TRUE)<<endl;
            if(res == news_to_be_checked->news_type){
                fact_checker->balance += reward;
                fact_checker->credibility += 0.05*(100 - fact_checker->credibility);
                res_voters += fact_checker->credibility;
                correct_voters.push_back(i);
            }
            else{
                fact_checker->balance -= 2*reward;
                fact_checker->credibility -= 5;
            }
            sum_credibility += fact_checker->credibility;
        }

        for(auto x: correct_voters){
            node[x]->balance += 10.0 / correct_voters.size();
        }

        cout<<"News "<<t<<"which is created by node "<<check_requester<<" is actually: "<<((news_to_be_checked->news_type == TRUE) ? "TRUE" : "FAKE")<<endl;
        cout<<"according to voters News "<<t<<" is: "<<((res_voters/sum_credibility)>0.5 ? ((news_to_be_checked->news_type == TRUE) ? "TRUE" : "FAKE") : ((news_to_be_checked->news_type == FAKE) ? "TRUE" : "FAKE"))<<endl<<endl;
    }
    
    cout<<"____________________________________________________________________________"<<endl;
    
    for(int i=0; i<n; i++){
    	cout<<"Node "<<i<<" has a creadibility score of "<<node[i]->credibility<<" and a balance of "<<node[i]->balance<<endl;
    }

    return 0;
}
