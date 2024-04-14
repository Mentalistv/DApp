#include <iostream>
#include <vector>
#include <string>
#include <time.h>
#include <unordered_set>
#include <random>
#include <iomanip>

using namespace std;

// SIMULATION PARAMETERS
#define NUMBER_OF_REQUESTS 1000
#define AMOUNT_AT_STAKE 0.2
#define FACT_CHECKERS_PERECNTAGE 0.3
#define CREDIBILITY_INC_MULTIPLIER 0.05
#define CREDIBILITY_DEC_MULTIPLIER 0.1
#define VERIFICATION_FEES 1
#define INITIAL_CREDIBILITY 30


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
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_real_distribution<double> distribution(lower, upper);

    return distribution(gen);
}

bool resultUsingProb(double prob, bool res){
    return ((double)rand() / RAND_MAX) < prob ? res : !res;
}

int main(int argc, char* argv[]){
    vector<Node *> node;
    int n;
    double p, q;

    n = atoi(argv[1]);
    q = atof(argv[2]);
    p = atof(argv[3]);

    for(int i=0; i<n; i++){
        Node* fact_checker;
        if(i < (1-q)*n){
            if(i < p*n){
                fact_checker = new Node(HONEST, 100, INITIAL_CREDIBILITY, 0.9, {"fd"});
            }
            else{
                fact_checker = new Node(HONEST, 100, INITIAL_CREDIBILITY, 0.7, {"fd"});
            }
        }
        else{
            fact_checker = new Node(MALICIOUS, 100, INITIAL_CREDIBILITY, 0, {"fd"});
        }
        node.push_back(fact_checker);
    }

    for(int t=0; t<NUMBER_OF_REQUESTS; t++){
        // selecting node which would put request for news verfication and got sufficient balance
        int check_requester = randomNumberGenerator(0, n);
        int count = 101;
        
        while(node[check_requester]->balance < VERIFICATION_FEES && count--)	
            check_requester = randomNumberGenerator(0, n);

        News* news_to_be_checked = new News(resultUsingProb(0.5, true) ? TRUE : FAKE, "dfsd", 10);
        node[check_requester]->balance -= VERIFICATION_FEES;

        double total_reward = 0;
        total_reward += VERIFICATION_FEES;

        int number_of_fact_checkers = FACT_CHECKERS_PERECNTAGE * n;
        unordered_set <int> fact_checkers;

        // selecting fact checkers
        while(fact_checkers.size() != number_of_fact_checkers){
            int num = randomNumberGenerator(0, n);

            if(check_requester != num && node[num]->balance >= AMOUNT_AT_STAKE)
                fact_checkers.insert(num);
        }

        double sum_credibility = 0;
        double voters_result = 0;
        vector<int> correct_voters;

        // voting and updating credibility
        for(auto i: fact_checkers){
            Node* fact_checker = node[i];
            NewsType res = resultUsingProb(fact_checker->prob, news_to_be_checked->news_type == TRUE) ? TRUE : FAKE;

            if(res == news_to_be_checked->news_type){
                fact_checker->credibility += CREDIBILITY_INC_MULTIPLIER * (100 - fact_checker->credibility);
                voters_result += fact_checker->credibility;
                correct_voters.push_back(i);
            }
            else{
                fact_checker->credibility -= CREDIBILITY_DEC_MULTIPLIER * fact_checker->credibility;
                fact_checker->balance -= AMOUNT_AT_STAKE;
            }

            sum_credibility += fact_checker->credibility;
        }

        // rewarding voters
        for(auto x: correct_voters){
            node[x]->balance += total_reward / correct_voters.size();
        }

        std::cout << std::fixed;
        std::cout << std::setprecision(3);  
        std::cout<<"News "<<t<<" which is created by node "<<check_requester<<" is actually: "<<((news_to_be_checked->news_type == TRUE) ? "TRUE" : "FAKE")<<endl;
        std::cout<<"According to voters News "<<t<<" is: "<<((voters_result/sum_credibility)>0.5 ? ((news_to_be_checked->news_type == TRUE) ? "TRUE" : "FAKE") : ((news_to_be_checked->news_type == FAKE) ? "TRUE" : "FAKE"))<<endl<<endl;
    }


    cout<<"____________________________________________________________________________"<<endl;
    cout<<"\t\tFINAL RESULTS"<<endl;
    cout<<"____________________________________________________________________________"<<endl;

    for(int i=0; i<n; i++){
    	cout<<"Node "<<i<<" has a creadibility score of "<<node[i]->credibility<<" and a balance of "<<node[i]->balance<<endl;
    }

    return 0;
}
