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

#define PROB_TRUSTWORTHY_HONEST 0.9
#define PROB_LESS_TRUSTWORTHY_HONEST 0.7

#define VERIFICATION_FEES 0.5
#define AMOUNT_AT_STAKE 0.02
#define INITIAL_CREDIBILITY 30
#define CREDIBILITY_INC_MULTIPLIER 0.05
#define CREDIBILITY_DEC_MULTIPLIER 0.1
#define EXPERTISE_MULTIPLIER 0.2
#define POS_WEIGHT 1

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
    int n, m, I;
    double p, q;

    if (argc != 5) {
        std::cout << "Usage: ./a.out <total nodes> <fraction of malicious> <fraction of more trustworthy honest nodes> <% wealth with malicious>" << endl;
        return 1;
    }

    n = atoi(argv[1]);
    q = atof(argv[2]);
    p = atof(argv[3]);
    m = atoi(argv[4]);

    float mal = (float) (m * (1 - q) * HONEST_NODE_INITIAL_BALANCE * n)/(float) (HONEST_NODE_INITIAL_BALANCE * q * n - m * q * n);
    I = floor(mal);

    for(int i=0; i<n; i++){
        Node* fact_checker;
        int checkers_expertise_index = randomNumberGenerator(0, expertise.size());
        string checkers_expertise = expertise[checkers_expertise_index];

        if(i < (1-q)*n){
            if(i < (1-q)*p*n){
                // Trustworthly honest nodes
                fact_checker = new Node(HONEST, HONEST_NODE_INITIAL_BALANCE, INITIAL_CREDIBILITY, PROB_TRUSTWORTHY_HONEST, checkers_expertise);
            }
            else{
                // less trustworthy honest nodes
                fact_checker = new Node(HONEST, HONEST_NODE_INITIAL_BALANCE, INITIAL_CREDIBILITY, PROB_LESS_TRUSTWORTHY_HONEST, checkers_expertise);
            }
        }
        else{
            // malicious nodes
            fact_checker = new Node(MALICIOUS, I, INITIAL_CREDIBILITY, 0, checkers_expertise);
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

        News* news_to_be_checked = new News(resultUsingProb(0.5, true) ? TRUE : FAKE, news_category, VERIFICATION_FEES);
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
                voters_result += EXPERTISE_MULTIPLIER * fact_checker->credibility;
                sum_credibility += EXPERTISE_MULTIPLIER * fact_checker->credibility;
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

    double honest_9_avg_balance = 0;
    double honest_9_avg_credibility = 0;

    double honest_7_avg_balance = 0;
    double honest_7_avg_credibility = 0;

    double malicious_avg_balance = 0;
    double malicious_avg_credibility = 0;


    for(int i=0; i<n; i++){
        Node* fact_checker = node[i];

        if(i < (1-q)*n){
            if(i < (1-q)*p*n){
                honest_9_avg_balance += fact_checker->balance;
                honest_9_avg_credibility += fact_checker->credibility;
            }
            else{
                honest_7_avg_balance += fact_checker->balance;
                honest_7_avg_credibility += fact_checker->credibility;
            }
        }
        else{
            malicious_avg_balance += fact_checker->balance;
            malicious_avg_credibility += fact_checker->credibility;
        }
    }
    
    honest_9_avg_balance /= (1-q)*p*n;
    honest_9_avg_credibility /= (1-q)*p*n;
    
    honest_7_avg_balance /= (1-q)*(1-p)*n;
    honest_7_avg_credibility /= (1-q)*(1-p)*n;

    malicious_avg_balance /= q*n; 
    malicious_avg_credibility /= q*n; 

    std::cout<<"Honest Nodes with 0.9 prob <===> "<<"avg balance = "<<honest_9_avg_balance<<"\t"<<"avg credibility = "<<honest_9_avg_credibility<<endl;
    std::cout<<"Honest Nodes with 0.7 prob <===> "<<"avg balance = "<<honest_7_avg_balance<<"\t"<<"avg credibility = "<<honest_7_avg_credibility<<endl;
    std::cout<<"Malicious nodes <===> "<<"avg balance = "<<malicious_avg_balance<<"\t"<<"avg credibility = "<<malicious_avg_credibility<<endl;

    return 0;
}
