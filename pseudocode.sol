// SPDX-License-Identifier: GPL-3.0
pragma solidity ^0.8.4;


contract fake_news{


    uint constant VERIFY_FEES = 0.9;
    uint constant VOTING_DEPOSIT = 0.7;
    uint constant VOTERS_NUM = 10;
    uint constant STAKE_MULTIPLIER = 10;
    uint constant CRDIBILITY_INC_MULTIPLIER = 0.5;


    struct News {
        uint status;
        uint vote_sum;
        uint vote_tot;  
        uint result;  
        uint reward; // 0 by default
        uint correct_votes; // 0 by default
        address[] volunteers;
        address[] voters; 
        uint[] votes;                                                   
    } 
    


    mapping(address => uint) public balances;
    mapping(address => uint) public credibility;
    mapping (uint => News) public news_data;


    // customer gets news verified by paying verification fee
    function verifyNews(address verifier_id, uint news_id) public {
        
        if(balances[verifier_id] >= VERIFY_FEES){
            balances[verifier_id] -= VERIFY_FEES;

            news_data[news_id].status = 1;
            news_data[news_id].reward += VERIFY_FEES;
        }
        
    }
    

    // potential fact checker volunteer to vote
    function volunteerToVote(address voter_id, uint news_id) public {
        if(balances[voter_id] >= VOTING_DEPOSIT){
            balances[voter_id] -= VOTING_DEPOSIT;

            news_data[news_id].volunteers.push(voter_id);
        }

    }


    // select fact checkers from the nodes who have volunteered
    function selectVoters(uint news_id) public {
        address[] memory tempData = news_data[news_id].volunteers;

        for (uint i = 0; i < VOTERS_NUM; i++) {
            uint index = uint(keccak256(abi.encodePacked(block.timestamp, msg.sender, i))) % tempData.length;
            news_data[news_id].voters[i] = tempData[index];
            news_data[news_id].votes[i] = 0;

            // Remove the selected element from array
            if (index < tempData.length - 1) {
                tempData[index] = tempData[tempData.length - 1];
            }

            tempData.pop();
        }
    }

    // voter votes on a news article 
    function vote(uint news_id, address voter_id, uint vote) public{

        // NOTE: e = news.category == voter.expertise ? EXPERTISE_MULTIPLIER : 0

        news_data[news_id].votes[voter_id] = vote; 
        news_data[news_id].vote_sum = vote*credibility[voter_id] + STAKE_MULTIPLIER*balances[voter_id] + e;
        news_data[news_id].vote_tot = credibility[voter_id] + STAKE_MULTIPLIER*balances[voter_id] + e;
    }


    // voting result are computed and reward are distributed
    function evaluate(uint news_id) public{

        if(news_data[news_id].vote_sum / news_data[news_id].vote_tot > 0.5){
            news_data[news_id].result = 1;
        }
        else{
            news_data[news_id].result = 0;
        }


        //update credibility
        for(uint i = 0; i < news_data[news_id].votes.length; i++){
            if(news_data[news_id].votes[i] == news_data[news_id].result){
                credibility[news_data[news_id].voters[i]] += CRDIBILITY_INC_MULTIPLIER*(100 - credibility[news_data[news_id].voters[i]]);
                news_data[news_id].correct_votes++; 
            }
            else{
                credibility[news_data[news_id].voters[i]] -= CRDIBILITY_INC_MULTIPLIER*(100 - credibility[news_data[news_id].voters[i]]); 
                news_data[news_id].reward += VOTING_DEPOSIT;
            }
        }


        //distribute reward  
        uint reward_per_voter = news_data[news_id].reward / news_data[news_id].correct_votes;
        for(uint i = 0; i < news_data[news_id].voters.length; i++){
            if(news_data[news_id].votes[i] == news_data[news_id].result){
                balances[news_data[news_id].voters[i]] += reward_per_voter;
            }
        }


    }

}