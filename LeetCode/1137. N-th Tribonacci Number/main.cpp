class Solution {
public:
    int tribonacci(int n) {
        if(n == 0) return 0;
        if(n == 1) return 1;
        if(n == 2) return 1;
        
        vector<int> fibbV(n + 1);
        fibbV[0] = 0;
        fibbV[1] = 1;
        fibbV[2] = 1;

        
        for(int i = 3; i < n + 1; ++i){
            fibbV[i] = fibbV[i - 3] + fibbV[i - 2] + fibbV[i - 1];
        }
        
        return fibbV[n];
    }
};