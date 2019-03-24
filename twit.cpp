#include<iostream>
#include<string.h>
#include<vector> 
#include<fstream>
#include<algorithm>
#include<cmath>
#include<dirent.h>

using namespace std;

int fn = 1;

vector<string> Get_pos_files()
{
	vector<string>words;
	ifstream fin1;
	fin1.open("value_sentiment.txt");
	string binary;
	fin1>>binary;
	ifstream fin2;
	fin2.open("Processed_text.txt");
	for(int i = 0; i < binary.length(); i++)
	{
		string sent;
		getline(fin2, sent);
		if(binary[i] == '1')
			words.push_back(sent);
	}
	fin1.close();
	fin2.close();
	return words;
}

vector<string> Get_neg_files()
{
	vector<string>words;
	ifstream fin1;
	fin1.open("value_sentiment.txt");
	string binary;
	fin1>>binary;
	ifstream fin2;
	fin2.open("Processed_text.txt");
	for(int i = 0; i < binary.length(); i++)
	{
		string sent;
		getline(fin2, sent);
		if(binary[i] == '0')
			words.push_back(sent);
	}
	fin1.close();
	fin2.close();
	return words;
}

vector<string> Get_test_data()
{
	vector<string>words;
	ifstream fin2;
	fin2.open("Processed_test_data.txt");
	while(fin2)
	{
		string sent;
		getline(fin2, sent);
		words.push_back(sent);
	}
	fin2.close();
	return words;
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
        {
            if(temp == "not")
                fn = 0;
            else
            processed.push_back(temp);
        }
    }
    return processed;
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
        matrix.push_back(sparse_Matrix(all_reviews[i], words));
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
    double variance = 0.0;
    for(int i = 0; i < v.size(); i++)
        variance = variance + (avg - v[i])*(avg - v[i]);
    variance /= v.size();
    return sqrt(variance);
}

vector<vector<double> > summByClass(vector<vector<int> > pos_dataset, vector<vector<int> > neg_dataset)
{
    vector<double> pos_mean;
    vector<double> pos_std_dev;

    for(int i = 0; i < pos_dataset[0].size(); i++)
    {
        vector<int> temp;
        for(int j = 0; j < pos_dataset.size(); j++)
            temp.push_back(pos_dataset[j][i]);
        pos_mean.push_back(mean(temp));
        pos_std_dev.push_back(std_dev(temp));
    }
    
    vector<double> neg_mean;
    vector<double> neg_std_dev;

    for(int i = 0; i < neg_dataset[0].size(); i++)
    {
        vector<int> temp;
        for(int j = 0; j < neg_dataset.size(); j++)
            temp.push_back(neg_dataset[j][i]);
        neg_mean.push_back(mean(temp));
        neg_std_dev.push_back(std_dev(temp));
    }

    vector<vector<double> > summaries;
    summaries.push_back(pos_mean);
    summaries.push_back(pos_std_dev);
    summaries.push_back(neg_mean);
    summaries.push_back(pos_std_dev);
    
    return summaries;
}


double calculate_probablity(double x, double mean, double std_dev)
{
    if(std_dev != 0 && x != 0)
    {
        double exponent = exp(-0.5 * pow((x - mean)/std_dev, 2));
        double den = std_dev*sqrt(2 * M_PI);
        return exponent/den;
    }
    else
       return 0;
}

vector<double> Calculate_Probablities_By_Class(vector<vector<double> > summaries, vector<int> input)
{
    vector<double> probablities;
    probablities.push_back(0);
    for(int i = 0; i < summaries[0].size(); i++)
    {  
        double mean = summaries[0][i];
        double std_dev = summaries[1][i];
        double prob = calculate_probablity(input[i], mean, std_dev);
        probablities[0]+=prob;       
    }
    probablities.push_back(0);
    for(int i = 0; i < summaries[0].size(); i++)
    {    
        double mean = summaries[2][i];
        double std_dev = summaries[3][i];
        double prob = calculate_probablity(input[i], mean, std_dev);
        probablities[1]+=prob;            
    }
    return probablities;
}

string predict(vector<vector<double> > summaries, vector<int> input)
{
    vector<double> probablities = Calculate_Probablities_By_Class(summaries, input);
    double best_prob = max(probablities[0], probablities[1]);
    if(fn)
    {
        if(best_prob == probablities[0])
            return "positive";
        else 
            return "negative";
    }
    else
    {
        if(!(best_prob == probablities[0]))
            return "positive";
        else 
            return "negative";
    }
}

double accuracy(vector<int> pred)
{
	ifstream fin;
	fin.open("value_test_sentiment.txt");
	string binary;// = "100010001011010";
	getline(fin, binary);
	int sum = 0;
	for(int i = 0; i < binary.length(); i++)
	{
        //cout<<i+1<<" "<<binary[i]<<"\t"<<pred[i]<<endl;
		if(binary[i] - 48 == pred[i])
			sum++;
	}
	cout<<sum<<endl;
    fin.close();
	return double(sum)/binary.length()*100;
}	

int main()
{
	vector<string> words = Words_array_from_file();
	vector<string> neg_revs = Get_neg_files();
	vector<string> pos_revs = Get_pos_files();
	vector<vector<int> > neg_sparse = sparse_2d(neg_revs, words);
	vector<vector<int> > pos_sparse = sparse_2d(pos_revs, words);
	vector<vector<double> > summaries = summByClass(pos_sparse, neg_sparse);

	vector<string> test = Get_test_data();
    /*vector<string> test;
    test.push_back("the boy was happy");
    test.push_back("the boy was not happy");
    test.push_back("the movie was not amazing");
    test.push_back("I did not have fun at the party");
    test.push_back("not boring session");
    test.push_back("the food was disgusting");
    test.push_back("i felt extremely uncomfortable");
    test.push_back("the car crashed and the man dies");
    test.push_back("raining and felt good");
    test.push_back("riding cycle and get hurt");
    test.push_back("green book");
    test.push_back("overjoyed with marks");
    test.push_back("annoyed with the show");
    test.push_back("not happy");
    test.push_back("fast and no time");
	vector<int> pred;
	for(int i = 0; i < test.size(); i++)
	{
		string rev = predict(summaries, sparse_Matrix(test[i], words));
		if(rev == "negative")
			pred.push_back(0);
		else
			pred.push_back(1);
	}
	cout<<accuracy(pred)<<"%"<<endl<<endl;*/

    while(1)
    {
        cout<<"Enter you tweet: "<<endl;
        string user;
        getline(cin, user);
        if(user == "-1")
            break;
        cout<<"Prediction: "<<predict(summaries, sparse_Matrix(user, words))<<endl<<endl;
    }

 	return 0;
}