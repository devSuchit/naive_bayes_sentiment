#include<iostream>
#include<string.h>
#include<vector> 
#include<fstream>
#include<algorithm>
#include<cmath>
#include <dirent.h>

using namespace std;

string Get_Review(string filename, string dir)
{
    ifstream fin;
    filename = dir+filename+".txt";
    fin.open(filename);
    string review;
    string word;
    if (!fin)
    	cout<<"File not found\n";
    while(fin)
    {
        getline(fin, word);
        review = review + word + "\n";
    }
    return review;
}

vector<string> Get_neg_Files()
{
	vector<string> reviews;
	DIR *d;
    char *p1,*p2;
    int ret;
    struct dirent *dir;
    d = opendir("neg_revs");
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            p1 = strtok(dir->d_name, ".");
            p2 = strtok(NULL, ".");
            if(p2 != NULL)
            {
                ret = strcmp(p2, "txt");
                if(ret == 0)
                {
                	string s(p1);
                    reviews.push_back(Get_Review(s, "neg_revs/"));
                }
            }

        }
        closedir(d);
    }
	return reviews;
}

vector<string> Get_pos_Files()
{
	vector<string> reviews;
	DIR *d;
    char *p1,*p2;
    int ret;
    struct dirent *dir;
    d = opendir("pos_revs");
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            p1 = strtok(dir->d_name, ".");
            p2 = strtok(NULL, ".");
            if(p2 != NULL)
            {
                ret = strcmp(p2, "txt");
                if(ret == 0)
                {
                	string s(p1);
                    reviews.push_back(Get_Review(s, "pos_revs/"));
                }
            }

        }
        closedir(d);
    }
	return reviews;
}

vector<string> Words_array_from_file()
{
    vector<string> words;
    ifstream fin;
    fin.open("All_Words.txt");
    string word;
    while(fin)
    {
        fin>>word;
        words.push_back(word);
    }
    words.pop_back();
    return words;
}

vector<string> Pre_Process(string sent)
{
    vector<string>processed;

    for(int i = 0; i < sent.length();i++)
    {
        string temp = "";

        while(isalnum(sent[i]))
            temp = temp + sent[i++];
        
        if(temp != "")
            processed.push_back(temp);
    }
    return processed;
}

string to_lower(string str) 
{
	string str1 = "";
	for(int i = 0; i < str.length(); i++)
  	{
  		if(str[i] <= 'Z' && str[i] >= 'A')
    		 str1 = str1 + char(str[i] - ('Z' - 'z'));
  		else
  		 str1 += str[i];
  	}	
  	return str1;
}

vector<int> sparse_Matrix(string sent, vector<string> &words)
{
    vector<string> sentence = Pre_Process(sent);
    vector<int> matrix (words.size());
    for(int i = 0; i < words.size(); i++)
    {
        for(int j = 0; j < sentence.size(); j++)
        {
        
            if(words[i] == sentence[j])
            {
                matrix[i] += 1;
                break;
            }
        }
    }
    return matrix;
}

vector<vector<int> > sparse_2d(vector<string> &all_reviews, vector<string> &words)
{
    vector<vector<int> > matrix;
    for(int i = 0; i < all_reviews.size(); i++)
        matrix.push_back(sparse_Matrix(all_reviews[0], words));
    return matrix;
}


double mean(vector<int> v)
{
    int sum = 0;
    for(int i = 0; i < v.size(); i++)
        sum+=v[i];
    return double(sum)/v.size();
}

double std_dev(vector<int> v)
{
    double avg = mean(v);
    double variance = 0;
    for(int i = 0; i < v.size(); i++)
        variance += pow((avg - v[i]), 2);
    variance /= v.size();
    cout<<sqrt(variance);
    return sqrt(variance);
}

vector<vector<int> > summByClass(vector<vector<int> > pos_dataset, vector<vector<int> > neg_dataset)
{
    vector<int> pos_mean;
    vector<int> pos_std_dev;

    for(int i = 0; i < pos_dataset[0].size(); i++)
    {
        vector<int> temp;
        for(int j = 0; j < pos_dataset.size(); j++)
            temp.push_back(pos_dataset[j][i]);
        pos_mean.push_back(mean(temp));
        pos_std_dev.push_back(std_dev(temp));
    }
    
    vector<int> neg_mean;
    vector<int> neg_std_dev;
    for(int i = 0; i < neg_dataset[0].size(); i++)
    {
        vector<int> temp;
        for(int j = 0; j < neg_dataset.size(); j++)
            temp.push_back(neg_dataset[j][i]);
        neg_mean.push_back(mean(temp));
        neg_std_dev.push_back(std_dev(temp));
    }

    vector<vector<int> > summaries;
    summaries.push_back(pos_mean);
    summaries.push_back(pos_std_dev);
    summaries.push_back(neg_mean);
    summaries.push_back(pos_std_dev);
    
    return summaries;
}


double calculate_probablity(double x, double mean, double std_dev)
{
    double exponent = exp(-0.5 * pow((x - mean)/std_dev, 2));
    double den = std_dev*sqrt(2 * M_PI);
    return exponent/den;
}

vector<int> Calculate_Probablities_By_Class(vector<vector<int> > summaries, vector<int> input)
{
    vector<int> probablities;
    probablities.push_back(1);
    //cout<<"Positive: "<<endl;
    for(int i = 0; i < summaries[0].size(); i++)
    {  
        double mean = summaries[0][i];
        double std_dev = summaries[1][i];
        //cout<<mean<<" "<<std_dev<<"\t";
        probablities[0]*=calculate_probablity(input[i], mean, std_dev);            
    }
    //cout<<"\n\nNegative: "<<endl;
    probablities.push_back(1);
    for(int i = 0; i < summaries[0].size(); i++)
    {    
        double mean = summaries[2][i];
        double std_dev = summaries[3][i];
        //cout<<mean<<"\t"<<std_dev;
        probablities[1]*=calculate_probablity(input[i], mean, std_dev);            
    }
    cout<<endl<<endl;
    return probablities;
}

int predict(vector<vector<int> > summaries, vector<int> input)
{
    vector<int> probablities = Calculate_Probablities_By_Class(summaries, input);
    int best_prob = max(probablities[0], probablities[1]);
    cout<<probablities[0]<<" "<<probablities[1]<<endl;
    if(best_prob == probablities[0])
        return 1;
    else 
        return 0;
}

int main()
{
	vector<string> words = Words_array_from_file();
	vector<string> neg_revs = Get_neg_Files();
	vector<string> pos_revs = Get_pos_Files();
	vector<vector<int> > neg_sparse = sparse_2d(neg_revs, words);
	vector<vector<int> > pos_sparse = sparse_2d(pos_revs, words);
	vector<vector<int> > summaries = summByClass(pos_sparse, neg_sparse);
	vector<int> sample = sparse_Matrix(Get_Review("64_1", ""), words);
	cout<<predict(summaries, sample)<<endl;
	//cout<<calculate_probablity(1, 2, 2.5);
 	return 0;
}
