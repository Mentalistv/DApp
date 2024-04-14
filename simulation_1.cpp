#include <iostream>
#include <vector>
#include <string>
#include <time.h>
#include <unordered_set>
#include <unordered_map>
#include <random>
#include <iomanip>

using namespace std;

// SIMULATION PARAMETERS
#define HONEST_NODE_INITIAL_BALANCE 100
#define MALICIOUS_NODE_INITIAL_BALANCE 15

#define NUMBER_OF_REQUESTS 1000
#define FACT_CHECKERS_PERECNTAGE 0.5

#define VERIFICATION_FEES 0.5
#define AMOUNT_AT_STAKE 0.02
#define INITIAL_CREDIBILITY 30
#define CREDIBILITY_INC_MULTIPLIER 0.05
#define CREDIBILITY_DEC_MULTIPLIER 0.1
#define POS_WEIGHT 0.5

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
    Node(NodeType node_type, double balance, double credibility, double prob, string expertise){
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
    string expertise;
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
    vector<string> expertise = {"ML", "Systems", "Theory", "Sports", "Politics"};
    int n;
    double p, q;

    n = atoi(argv[1]);
    q = atof(argv[2]);
    p = atof(argv[3]);

    for(int i=0; i<n; i++){
        Node* fact_checker;
        int checkers_expertise_index = randomNumberGenerator(0, expertise.size());
        string checkers_expertise = expertise[checkers_expertise_index];

        if(i < (1-q)*n){
            if(i < p*n){
                fact_checker = new Node(HONEST, HONEST_NODE_INITIAL_BALANCE, INITIAL_CREDIBILITY, 0.9, checkers_expertise);
            }
            else{
                fact_checker = new Node(HONEST, HONEST_NODE_INITIAL_BALANCE, INITIAL_CREDIBILITY, 0.7, checkers_expertise);
            }
        }
        else{
            fact_checker = new Node(MALICIOUS, MALICIOUS_NODE_INITIAL_BALANCE, INITIAL_CREDIBILITY, 0, checkers_expertise);
        }
        node.push_back(fact_checker);
    }

    for(int t=0; t<NUMBER_OF_REQUESTS; t++){
        // selecting node which would put request for news verfication and got sufficient balance
        int check_requester = randomNumberGenerator(0, n);
        int count = 101;
        
        while(node[check_requester]->balance < VERIFICATION_FEES && count--)	
            check_requester = randomNumberGenerator(0, n);

        // generating news
        int news_category_index = randomNumberGenerator(0, expertise.size());
        string news_category = expertise[news_category_index];

        News* news_to_be_checked = new News(resultUsingProb(0.5, true) ? TRUE : FAKE, news_category, 10);
        node[check_requester]->balance -= VERIFICATION_FEES;

        // incent for the voters
        double total_reward = 0;
        total_reward += VERIFICATION_FEES;

        // selecting fact checkers
        int number_of_fact_checkers = FACT_CHECKERS_PERECNTAGE * (n-1);
        unordered_set <int> fact_checkers;

        while(fact_checkers.size() != number_of_fact_checkers){
            int num = randomNumberGenerator(0, n);

            if(check_requester != num && node[num]->balance >= AMOUNT_AT_STAKE)
                fact_checkers.insert(num);
        }

        // voting
        unordered_map <int, NewsType> res;
        int total_fact_checkers_balance = 0;
        double voters_result = 0;
        double sum_credibility = 0;

        for(auto i: fact_checkers){
            Node* fact_checker = node[i];
            NewsType temp_res = resultUsingProb(fact_checker->prob, news_to_be_checked->news_type == TRUE) ? TRUE : FAKE;

            res[i] = temp_res;

            // deciding result based on the credibility and balance of the voters
            if(temp_res == TRUE){
                voters_result += fact_checker->credibility + fact_checker->balance * POS_WEIGHT;
            }

            sum_credibility += fact_checker->credibility;

            // expertise in the topic gets more credibility 
            if(fact_checker->expertise == news_to_be_checked->news_category){
                voters_result += 20;
                sum_credibility += 20;
            }

            total_fact_checkers_balance += fact_checker->balance;

            // voters putting deposists
            fact_checker->balance -= AMOUNT_AT_STAKE;
            total_reward += AMOUNT_AT_STAKE;
        }

        NewsType result_after_voting = (voters_result/(sum_credibility + POS_WEIGHT * total_fact_checkers_balance)) > 0.5 ? TRUE : FAKE;

        // updating credibility 
        vector<int> correct_voters;
        for(auto i: fact_checkers){
            Node* fact_checker = node[i];

            if(res[i] == result_after_voting){
                fact_checker->credibility += CREDIBILITY_INC_MULTIPLIER * (100 - fact_checker->credibility);
                correct_voters.push_back(i);
            }
            else{
                fact_checker->credibility -= CREDIBILITY_DEC_MULTIPLIER * fact_checker->credibility;
            }

        }

        // rewarding voters
        for(auto x: correct_voters){
                node[x]->balance += total_reward / correct_voters.size();
        }

        std::cout<<"According to voters News "<<t<<" is: "<<result_after_voting<<endl;
    }
 

    std::cout<<"\n_______________________________________________________________________"<<endl;
    std::cout<<"\t\tFINAL RESULTS"<<endl;
    std::cout<<"_______________________________________________________________________\n"<<endl;

    std::cout << std::fixed;
    std::cout << std::setprecision(3);  
    for(int i=0; i<n; i++){
    	std::cout<<"Node "<<i<<" has a creadibility score of "<<node[i]->credibility<<" and a balance of "<<node[i]->balance<<endl;
    }

    return 0;
}
