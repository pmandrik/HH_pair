
#include "LHEF.h"

 class LHEEvent {
    public:
    void Reset(){
      weights.clear();
      weights_v.clear();
    }

    void GetPDFErrors( double & weight_pdf_up, double & weight_pdf_down, const double weight, const vector<int> pdf_indexes ) {
      // PDF errors
      double mean = 0;
      for( int index : pdf_indexes )
        mean += weights_v.at( index );
      mean /= pdf_indexes.size();

      double sigma_pdf_2 = 0;
      for( int index : pdf_indexes )
        sigma_pdf_2 += pow( mean - weights_v[ index ], 2 ) / ( pdf_indexes.size() - 1 );

      weight_pdf_up   = weight + sqrt(sigma_pdf_2);
      weight_pdf_down = weight - sqrt(sigma_pdf_2);
    }
    
    map<string, float> weights;
    vector<float> weights_v;
  };

  class LHEReader {
    public:
    LHEReader(){
      file = NULL;
      weight_open_pattern = "<wgt id='";
      weight_middle_pattern = "'>";
      weight_exit_pattern = "</wgt>";
    };

    void Init(string file_name){
      file = fopen(file_name.c_str(), "r");
      if(file == NULL){
        cout << "mRoot::LHEReader.Init(): cant open file" << file_name << endl;; 
        return;
      }

      char * line = NULL;
      size_t len = 0;
      size_t read;
      header.clear();
      footer.clear();

      const char * tag;
      while((read = getline(&line, &len, file)) != -1) {
        header.emplace_back( line );
        if(strstr(line, "</init>") == NULL) continue;
        break;
      }
    }

    bool ReadEvent(){
      if(file == NULL) return false;
      
      event.Reset();
      /* FIXME one time reader */
      const char * tag, * subtag;
      char * line = NULL;
      size_t len = 0;
      size_t read;
      while((read = getline(&line, &len, file)) != -1) {
        tag = strstr(line, weight_open_pattern.c_str() );
        if(tag != NULL){
          subtag = strstr(tag, weight_middle_pattern.c_str());
          //strncpy(buffer, tag+9, subtag-tag-9);
          //buffer[subtag-tag-9] = '\0';
          //string weight_id = string( buffer );

          tag = strstr(tag, weight_exit_pattern.c_str() );
          subtag += 2;
          strncpy(buffer, subtag, tag-subtag);
          buffer[tag-subtag] = '\0';
          float weight = atof( buffer );

          // msg(weight_id, weight);
          //event.weights[ weight_id ] = weight;
          event.weights_v.push_back( weight );
          continue;
        }


        if(strstr(line, "</event>") != NULL) break;
        if(strstr(line, "</LesHouchesEvents>") != NULL){
          file = NULL;
          return false;
        }
      }

      // event.Init( event_str );
      return true;
    }

    FILE * file;
    LHEEvent event;
    vector<string> event_str;
    vector<string> header, footer;
    char buffer[500];

    string weight_open_pattern, weight_exit_pattern, weight_middle_pattern;
  };

















