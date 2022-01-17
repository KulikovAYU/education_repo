/**
 * Definition for singly-linked list.
 * struct ListNode {
 *     int val;
 *     ListNode *next;
 *     ListNode() : val(0), next(nullptr) {}
 *     ListNode(int x) : val(x), next(nullptr) {}
 *     ListNode(int x, ListNode *next) : val(x), next(next) {}
 * };
 */
class Solution {
public:
    ListNode* deleteDuplicates(ListNode* head) {
        if(!head)
            return nullptr;
        
      TraverseList(head->next, head);  
        
      return head;  
    }
    
    
    void TraverseList(ListNode* curr, ListNode* prev){
        
        if(!curr || !prev)
            return;
        
      
        auto RemoveNode = [](ListNode* lCurr, ListNode* lPrev)
        {
            ListNode* lnext = lCurr->next;
            lCurr->next = nullptr;
            lPrev->next = lnext;
        };
        
        if(curr->val == prev->val){
            RemoveNode(curr, prev);
            TraverseList(prev->next, prev);
        }else{
            TraverseList(curr->next, curr); 
        }
    }
    
};