#include<SDL.h>
#include<SDL_ttf.h>
#include<string>
#include<vector>
#include<fstream>


class TextInput{
    private:
        struct properties{
            SDL_Rect Dimensions;
            TTF_Font* gFont;
            char* FontFamily;
            int FontStyle;
            int FontSize;
            int direction;
            SDL_Color fg;
            SDL_Color bg;
            int CharSpacing;
            int LineSpacing;
        };
        struct LinePart{
            std::string content;
            properties settings;
        };
        struct textinputs{
            std::vector<LinePart> innertexts;
            int alignment;
            int maxHeight;
            int maxWidth;
            int maxCharSpacing;
            int maxLineSpacing;
        };
        struct blinkerstruct{
            SDL_Rect BlinkerPos;
            int num;
            int blinkersurfacelinepartindex;
            int linepartindex;
        };
        bool blinker;
        blinkerstruct blinkervalues;
        properties WholeSettings;
        std::vector<textinputs> Lines;
        int alignment;
        bool active;
        std::string selected_text;
        SDL_Cursor * hovercursor;
    public:
        TextInput(SDL_Rect* dimen,int font_size){

            //OVERALL SETTINGS FOR THE TEXT-INPUT (ALL SURFACES INCLUDED)
            WholeSettings.FontSize = font_size;
            WholeSettings.Dimensions = {dimen->x+5,dimen->y+5,dimen->w-10,dimen->h-10};
            WholeSettings.gFont = TTF_OpenFont("times_new_roman.ttf",WholeSettings.FontSize);
            WholeSettings.LineSpacing = TTF_FontLineSkip(WholeSettings.gFont);
            WholeSettings.FontFamily = const_cast<char *>(TTF_FontFaceFamilyName(WholeSettings.gFont));
            WholeSettings.FontStyle = TTF_GetFontStyle(WholeSettings.gFont);
            WholeSettings.CharSpacing = TTF_GetFontKerning(WholeSettings.gFont);
            WholeSettings.fg = {0,0,0,0};
            WholeSettings.bg = {255,255,255,255};
            WholeSettings.direction = TTF_DIRECTION_LTR;

            //BLINKER + MOUSEPOS + SELECTED TEXT INTERFACE
            active = true;
            blinker = true;
            int neww;
            int newh;
            TTF_SizeUTF8(WholeSettings.gFont,"|",&neww,&newh);
            blinkervalues.BlinkerPos = {WholeSettings.Dimensions.x,WholeSettings.Dimensions.y,neww,WholeSettings.FontSize};
            blinkervalues.num = 0;
            blinkervalues.blinkersurfacelinepartindex = 0;
            blinkervalues.linepartindex = -1;
            selected_text = "";
            hovercursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);

            //SURFACE-WISE SETTINGS
            LinePart temp;
            temp.content = "";
            temp.settings = WholeSettings;
            TTF_SizeUTF8(WholeSettings.gFont,temp.content.c_str(),&temp.settings.Dimensions.w,&temp.settings.Dimensions.h);
            //LINE-WISE SETTINGS
            textinputs temp2;
            temp2.innertexts.push_back(temp);
            temp2.alignment = TTF_GetFontWrappedAlign(WholeSettings.gFont);
            temp2.maxHeight = temp.settings.Dimensions.h;
            temp2.maxWidth = temp.settings.Dimensions.w;
            temp2.maxCharSpacing = WholeSettings.CharSpacing;
            temp2.maxLineSpacing = WholeSettings.LineSpacing;

            //ALL-LINES ! #FULL-TEXT-INPUT
            Lines.push_back(temp2);

            //GLOBAL ALIGNMENT
            alignment = TTF_GetFontWrappedAlign(WholeSettings.gFont);
        }

        bool check_overflow_horizontal(textinputs single_line){
            if (single_line.maxWidth>WholeSettings.Dimensions.w){
                return true;
            }
            return false;
        }
        
        bool check_overflow_vertical(){
            int total_height=0;
            int prev_spacing=0;
            for (textinputs a: Lines){
                total_height += a.maxHeight;
                if (a.maxLineSpacing>prev_spacing){
                    prev_spacing = a.maxLineSpacing;
                    total_height += a.maxLineSpacing;
                }
                else{
                    total_height+=prev_spacing;
                    prev_spacing = a.maxLineSpacing;
                }
            }
            if (total_height>WholeSettings.Dimensions.h){
                return true;
            }
            return false;
        }

        void garbage_collector(){
            for (textinputs a: Lines){
                if (a.innertexts.size()!=1){
                    int partofline=0;
                    for (LinePart b: a.innertexts){
                        if (b.content==""){
                            a.innertexts.erase(a.innertexts.begin() + partofline);
                        }
                        partofline++;
                    }
                }
            }
        }

        void render(SDL_Renderer * renderer) {
            int lineno = 0;
            SDL_Texture* texture = NULL;
            SDL_Surface* temp_surface = NULL;
            int maxheight=0;

            // Iterate over each line (textinputs)
            for (textinputs a : Lines) {
                if (lineno==0){
                    maxheight+=a.maxHeight;
                }
                else{
                    maxheight+=a.maxHeight+Lines[lineno-1].maxLineSpacing;
                }
                if (maxheight>WholeSettings.Dimensions.h){
                    break;
                }
                // Render each LinePart in the current line
                for (LinePart b : a.innertexts) {
                    temp_surface = TTF_RenderUTF8_Blended(b.settings.gFont, b.content.c_str(), b.settings.fg);
                    texture = SDL_CreateTextureFromSurface(renderer,temp_surface);
                    SDL_RenderCopy(renderer,texture,NULL,&b.settings.Dimensions);
                     // Free after each inner blit
                }
                lineno++;
            }
            SDL_FreeSurface(temp_surface);
        }



        void renderblinker(SDL_Renderer* renderer){
            SDL_Color color = {0,0,0,255};
            SDL_Surface* temp_surface = TTF_RenderUTF8_Blended(WholeSettings.gFont,"|",color);
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer,temp_surface);
            SDL_RenderCopy(renderer,texture,NULL,&blinkervalues.BlinkerPos);
        }

        void putthetext(char a){
            if (blinkervalues.linepartindex==-1){
                Lines[blinkervalues.num].innertexts[blinkervalues.blinkersurfacelinepartindex].content += a;
            }
            else if (blinkervalues.linepartindex<Lines[blinkervalues.num].innertexts[blinkervalues.blinkersurfacelinepartindex].content.size()-1){
                Lines[blinkervalues.num].innertexts[blinkervalues.blinkersurfacelinepartindex].content.insert(blinkervalues.linepartindex+1, 1, a);
            }
            else{
                Lines[blinkervalues.num].innertexts[blinkervalues.blinkersurfacelinepartindex].content += a;
            }
            int linechecker = blinkervalues.num;
            int wbhai,hbhai;
            std::string apple = "";
            apple += a;
            TTF_SizeUTF8(Lines[blinkervalues.num].innertexts[blinkervalues.blinkersurfacelinepartindex].settings.gFont,apple.c_str(),&wbhai,&hbhai);
            Lines[blinkervalues.num].maxWidth += wbhai + Lines[blinkervalues.num].innertexts[blinkervalues.blinkersurfacelinepartindex].settings.CharSpacing;
            Lines[blinkervalues.num].innertexts[blinkervalues.blinkersurfacelinepartindex].settings.Dimensions.w += wbhai + Lines[blinkervalues.num].innertexts[blinkervalues.blinkersurfacelinepartindex].settings.CharSpacing;
            int temp_max_height = Lines[blinkervalues.num].maxHeight;
            int temp_max_charspacing = Lines[blinkervalues.num].maxCharSpacing;
            int temp_max_linespacing = Lines[blinkervalues.num].maxLineSpacing;
            if (Lines[blinkervalues.num].maxHeight < Lines[blinkervalues.num].innertexts[blinkervalues.blinkersurfacelinepartindex].settings.Dimensions.h){
                Lines[blinkervalues.num].maxHeight = Lines[blinkervalues.num].innertexts[blinkervalues.blinkersurfacelinepartindex].settings.Dimensions.h;
            }
            if (Lines[blinkervalues.num].maxCharSpacing < Lines[blinkervalues.num].innertexts[blinkervalues.blinkersurfacelinepartindex].settings.CharSpacing){
                Lines[blinkervalues.num].maxCharSpacing = Lines[blinkervalues.num].innertexts[blinkervalues.blinkersurfacelinepartindex].settings.CharSpacing;
            }
            if (Lines[blinkervalues.num].maxLineSpacing < Lines[blinkervalues.num].innertexts[blinkervalues.blinkersurfacelinepartindex].settings.LineSpacing){
                Lines[blinkervalues.num].maxLineSpacing = Lines[blinkervalues.num].innertexts[blinkervalues.blinkersurfacelinepartindex].settings.LineSpacing;
            }
            bool maggi = false;
            while (check_overflow_horizontal(Lines[linechecker])){
                LinePart temp;
                if (maggi){
                    temp.content = Lines[linechecker-1].innertexts[Lines[linechecker-1].innertexts.size()-1].content[Lines[linechecker-1].innertexts[Lines[linechecker-1].innertexts.size()-1].content.size()-1];
                }
                else{
                    temp.content = a;
                }
                maggi = true;  
                temp.settings = Lines[linechecker].innertexts.back().settings;
                TTF_SizeUTF8(temp.settings.gFont,temp.content.c_str(),&temp.settings.Dimensions.w,&temp.settings.Dimensions.h);
                temp.settings.Dimensions.w += temp.settings.CharSpacing;
                temp.settings.Dimensions.x = Lines[blinkervalues.num].innertexts[0].settings.Dimensions.x;
                temp.settings.Dimensions.y = Lines[blinkervalues.num].innertexts[0].settings.Dimensions.y + Lines[blinkervalues.num].maxHeight + Lines[blinkervalues.num].maxLineSpacing;
                if (linechecker==Lines.size()-1){  
                    textinputs temp2;
                    temp2.innertexts.push_back(temp);
                    temp2.alignment = TTF_GetFontWrappedAlign(temp.settings.gFont);
                    temp2.maxHeight = temp.settings.Dimensions.h;
                    temp2.maxWidth = temp.settings.Dimensions.w;
                    temp2.maxCharSpacing = temp.settings.CharSpacing;
                    temp2.maxLineSpacing = temp.settings.LineSpacing;
                    Lines.push_back(temp2);
                }
                else{
                    Lines[linechecker+1].innertexts.insert(Lines[linechecker+1].innertexts.begin(),temp);
                    int wbbhai,hbbhai;
                    TTF_SizeUTF8(Lines[linechecker].innertexts[0].settings.gFont,temp.content.c_str(),&wbbhai,&hbbhai);
                    Lines[linechecker+1].maxWidth += temp.settings.Dimensions.w + temp.settings.CharSpacing;
                    if (Lines[linechecker+1].maxHeight < Lines[linechecker+1].innertexts[0].settings.Dimensions.h){
                        Lines[linechecker+1].maxHeight = Lines[linechecker+1].innertexts[0].settings.Dimensions.h;
                    }
                    if (Lines[linechecker+1].maxCharSpacing < Lines[linechecker+1].innertexts[0].settings.CharSpacing){
                        Lines[linechecker+1].maxCharSpacing = Lines[linechecker+1].innertexts[0].settings.CharSpacing;
                    }
                    if (Lines[linechecker+1].maxLineSpacing < Lines[linechecker+1].innertexts[0].settings.LineSpacing){
                        Lines[linechecker+1].maxLineSpacing = Lines[linechecker+1].innertexts[0].settings.LineSpacing;
                    }
                }
                Lines[linechecker].innertexts.back().content.erase(Lines[linechecker].innertexts[Lines[linechecker].innertexts.size()-1].content.size()-1);
                std::string app2le = "";
                app2le += a;
                TTF_SizeUTF8(Lines[blinkervalues.num].innertexts[blinkervalues.blinkersurfacelinepartindex].settings.gFont,app2le.c_str(),&wbhai,&hbhai);
                Lines[blinkervalues.num].maxWidth -= wbhai + Lines[blinkervalues.num].innertexts[blinkervalues.blinkersurfacelinepartindex].settings.CharSpacing;
                Lines[blinkervalues.num].innertexts[blinkervalues.blinkersurfacelinepartindex].settings.Dimensions.w -= (wbhai + Lines[blinkervalues.num].innertexts[blinkervalues.blinkersurfacelinepartindex].settings.CharSpacing);
                Lines[blinkervalues.num].maxCharSpacing = temp_max_charspacing;
                Lines[blinkervalues.num].maxLineSpacing = temp_max_linespacing;
                Lines[blinkervalues.num].maxHeight = temp_max_height;
                bool maggi = false;
                linechecker++;
            }
            if (maggi){
                int wbbhai,hbbhai;
                std::string apple = "";
                apple += Lines[linechecker].innertexts[0].content[0];
                TTF_SizeUTF8(Lines[linechecker].innertexts[0].settings.gFont,apple.c_str(),&wbbhai,&hbbhai);
                Lines[linechecker].maxWidth += wbbhai + Lines[linechecker].innertexts[0].settings.CharSpacing;
                if (Lines[linechecker].maxHeight < Lines[linechecker].innertexts[0].settings.Dimensions.h){
                    Lines[linechecker].maxHeight = Lines[linechecker].innertexts[0].settings.Dimensions.h;
                }
                if (Lines[linechecker].maxCharSpacing < Lines[linechecker].innertexts[0].settings.CharSpacing){
                    Lines[linechecker].maxCharSpacing = Lines[linechecker].innertexts[0].settings.CharSpacing;
                }
                if (Lines[linechecker].maxLineSpacing < Lines[linechecker].innertexts[0].settings.LineSpacing){
                    Lines[linechecker].maxLineSpacing = Lines[linechecker].innertexts[0].settings.LineSpacing;
                }
                if (blinkervalues.blinkersurfacelinepartindex==Lines[blinkervalues.num].innertexts.size()-1 && blinkervalues.linepartindex==Lines[blinkervalues.num].innertexts[blinkervalues.blinkersurfacelinepartindex].content.size()-1){
                    move_blinker(1);
                }
            }
            move_blinker(1);

        }

        void removethetext() {
            int linechecker = blinkervalues.num;
            int blinkerpart = blinkervalues.blinkersurfacelinepartindex;
            int linepart = blinkervalues.linepartindex;
            int alpha = 0;
            move_blinker(-1);
            if (linechecker==0 && linepart==-1){
                return;
            }
            else if (linepart==-1 && linechecker!=0){
                Lines[linechecker - 1].innertexts.back().content += "b";
                bool angel = true;
                int fw,fh;
                std::string temp_lol = "";

                while (angel) {
                    Lines[linechecker].innertexts.back().content+="b";
                    if (!check_overflow_horizontal(Lines[linechecker])) {
                        angel = false;
                    }
                    Lines[linechecker].innertexts.back().content.erase(Lines[linechecker].innertexts.back().content.size() - 1);
                    if (Lines[linechecker].innertexts.back().content==""){
                        Lines[linechecker - 1].innertexts.back().content.erase(Lines[linechecker - 1].innertexts.back().content.size()-1);
                        break;
                    }
                    //if (linepart!=-1){
                    //    Lines[linechecker].innertexts[blinkerpart].content.erase(linepart);
                    //    alpha = 1;
                    //}
                    bool removeprev = false;
                    while (!check_overflow_horizontal(Lines[linechecker - 1])) {
                        removeprev = true;
                        Lines[linechecker - 1].innertexts.back().content.erase(Lines[linechecker - 1].innertexts.back().content.size() - 1);
                        LinePart temporary = Lines[linechecker].innertexts[0];
                        temporary.settings.Dimensions.x = Lines[linechecker-1].innertexts.back().settings.Dimensions.x + Lines[linechecker-1].innertexts.back().settings.Dimensions.w + Lines[linechecker-1].innertexts.back().settings.CharSpacing;
                        temporary.settings.Dimensions.y = Lines[linechecker-1].innertexts.back().settings.Dimensions.y;
                        temporary.content = Lines[linechecker].innertexts[0].content[0];
                        TTF_SizeUTF8(temporary.settings.gFont,temporary.content.c_str(),&temporary.settings.Dimensions.w,&temporary.settings.Dimensions.h);
                        Lines[linechecker - 1].innertexts.push_back(temporary);
                        Lines[linechecker - 1].maxWidth += temporary.settings.Dimensions.w + temporary.settings.CharSpacing;
                        Lines[linechecker].maxWidth -= (temporary.settings.Dimensions.w + temporary.settings.CharSpacing);
                        Lines[linechecker].innertexts[0].settings.Dimensions.w -= (temporary.settings.Dimensions.w + temporary.settings.CharSpacing);
                        if (Lines[linechecker - 1].maxHeight<temporary.settings.Dimensions.h){
                            Lines[linechecker - 1].maxHeight = temporary.settings.Dimensions.h;
                        }
                        if (Lines[linechecker - 1].maxCharSpacing<temporary.settings.CharSpacing){
                            Lines[linechecker - 1].maxCharSpacing = temporary.settings.CharSpacing;
                        }
                        if (Lines[linechecker - 1].maxLineSpacing<temporary.settings.LineSpacing){
                            Lines[linechecker - 1].maxLineSpacing = temporary.settings.LineSpacing;
                        }
                        Lines[linechecker].innertexts[0].content.erase(0, 1);
                        Lines[linechecker - 1].innertexts.back().content += "b";
                        garbage_collector();
                    }
                    Lines[linechecker - 1].innertexts.back().content.erase(Lines[linechecker - 1].innertexts.back().content.size()-1);
                    if (angel==true){
                        Lines[linechecker].innertexts.back().content+="b";
                    }
                    int hi = 0;
                    for (LinePart ab: Lines[linechecker].innertexts){
                        if (ab.content!=""){
                            hi++;
                        }
                    }
                    if (hi==0){
                        Lines.erase(Lines.begin()+linechecker);
                    }
                    linechecker++;
                    alpha = 0;
                    linepart = 0;
                }

            }
            else if (Lines.size()!=1){
                Lines[linechecker].innertexts.back().content += "b";
                bool angel = true;
                if (!check_overflow_horizontal(Lines[linechecker])){
                    angel = false;
                    Lines[linechecker].innertexts.back().content.erase(Lines[linechecker].innertexts.back().content.size() - 1);
                }
                int fw,fh;
                std::string gojo = Lines[linechecker].innertexts[blinkerpart].content.substr(linepart,1);
                TTF_SizeUTF8(Lines[linechecker].innertexts[blinkerpart].settings.gFont,gojo.c_str(),&fw,&fh);
                Lines[linechecker].innertexts[blinkerpart].settings.Dimensions.w -= (fw + Lines[linechecker].innertexts[blinkerpart].settings.CharSpacing);
                Lines[linechecker].innertexts[blinkerpart].content.erase(linepart);

                std::string temp_lol = "";
                linechecker++;

                while (angel) {
                    Lines[linechecker].innertexts.back().content+="b";
                    if (!check_overflow_horizontal(Lines[linechecker])) {
                        angel = false;
                    }
                    Lines[linechecker].innertexts.back().content.erase(Lines[linechecker].innertexts.back().content.size() - 1);
                    //if (linepart!=-1){
                    //    Lines[linechecker].innertexts[blinkerpart].content.erase(linepart);
                    //    alpha = 1;
                    //}
                    bool removeprev = false;
                    while (!check_overflow_horizontal(Lines[linechecker - 1])) {
                        removeprev = true;
                        Lines[linechecker - 1].innertexts.back().content.erase(Lines[linechecker - 1].innertexts.back().content.size() - 1);
                        LinePart temporary = Lines[linechecker].innertexts[0];
                        temporary.settings.Dimensions.x = Lines[linechecker-1].innertexts.back().settings.Dimensions.x + Lines[linechecker-1].innertexts.back().settings.Dimensions.w + Lines[linechecker-1].innertexts.back().settings.CharSpacing;
                        temporary.settings.Dimensions.y = Lines[linechecker-1].innertexts.back().settings.Dimensions.y;
                        temporary.content = Lines[linechecker].innertexts[0].content[0];
                        TTF_SizeUTF8(temporary.settings.gFont,temporary.content.c_str(),&temporary.settings.Dimensions.w,&temporary.settings.Dimensions.h);
                        Lines[linechecker - 1].innertexts.push_back(temporary);
                        Lines[linechecker - 1].maxWidth += temporary.settings.Dimensions.w + temporary.settings.CharSpacing;
                        Lines[linechecker].maxWidth -= (temporary.settings.Dimensions.w + temporary.settings.CharSpacing);
                        Lines[linechecker].innertexts[0].settings.Dimensions.w -= (temporary.settings.Dimensions.w + temporary.settings.CharSpacing);
                        if (Lines[linechecker - 1].maxHeight<temporary.settings.Dimensions.h){
                            Lines[linechecker - 1].maxHeight = temporary.settings.Dimensions.h;
                        }
                        if (Lines[linechecker - 1].maxCharSpacing<temporary.settings.CharSpacing){
                            Lines[linechecker - 1].maxCharSpacing = temporary.settings.CharSpacing;
                        }
                        if (Lines[linechecker - 1].maxLineSpacing<temporary.settings.LineSpacing){
                            Lines[linechecker - 1].maxLineSpacing = temporary.settings.LineSpacing;
                        }
                        Lines[linechecker].innertexts[0].content.erase(0, 1);
                        Lines[linechecker - 1].innertexts.back().content += "b";
                        garbage_collector();
                    }
                    Lines[linechecker - 1].innertexts.back().content.erase(Lines[linechecker - 1].innertexts.back().content.size()-1);
                    if (angel==true){
                        Lines[linechecker].innertexts.back().content+="b";
                    }
                    linechecker++;
                    alpha = 0;
                    linepart = 0;
                }

            }
            else{
                int fw,fh;
                std::string gojo = Lines[linechecker].innertexts[blinkerpart].content.substr(linepart,1);
                TTF_SizeUTF8(Lines[linechecker].innertexts[blinkerpart].settings.gFont,gojo.c_str(),&fw,&fh);
                Lines[linechecker].innertexts[blinkerpart].settings.Dimensions.w -= (fw + Lines[linechecker].innertexts[blinkerpart].settings.CharSpacing);
                Lines[linechecker].innertexts[blinkerpart].content.erase(linepart);
            }
        }
   

        void move_blinker(int value){
            if (value==1){
                if (blinkervalues.linepartindex==(Lines[blinkervalues.num].innertexts[blinkervalues.blinkersurfacelinepartindex].content.size()-1)){
                    if (blinkervalues.blinkersurfacelinepartindex==(Lines[blinkervalues.num].innertexts.size()-1)){
                        blinkervalues.blinkersurfacelinepartindex = 0;
                        blinkervalues.linepartindex = -1;
                        blinkervalues.num+=1;
                        blinkervalues.BlinkerPos.x  = Lines[blinkervalues.num].innertexts[0].settings.Dimensions.x;
                        blinkervalues.BlinkerPos.y  = Lines[blinkervalues.num].innertexts[0].settings.Dimensions.y;
                    }
                    else{
                        std::string justtemp;
                        int wagain,hagain;
                        blinkervalues.blinkersurfacelinepartindex+=1;
                        blinkervalues.linepartindex = 0;
                        justtemp.push_back(Lines[blinkervalues.num].innertexts[blinkervalues.blinkersurfacelinepartindex].content[0]);
                        TTF_SizeUTF8(Lines[blinkervalues.num].innertexts[blinkervalues.blinkersurfacelinepartindex].settings.gFont,justtemp.c_str(),&wagain,&hagain);
                        blinkervalues.BlinkerPos.x +=( wagain + (Lines[blinkervalues.num].innertexts[blinkervalues.blinkersurfacelinepartindex-1].settings.CharSpacing)/2 + (Lines[blinkervalues.num].innertexts[blinkervalues.blinkersurfacelinepartindex].settings.CharSpacing)/2);
                    }
                }
                else{
                    std::string justtemp;
                    int wagain,hagain;
                    blinkervalues.linepartindex +=1;
                    justtemp.push_back(Lines[blinkervalues.num].innertexts[blinkervalues.blinkersurfacelinepartindex].content[blinkervalues.linepartindex]);
                    TTF_SizeUTF8(Lines[blinkervalues.num].innertexts[blinkervalues.blinkersurfacelinepartindex].settings.gFont,justtemp.c_str(),&wagain,&hagain);
                    blinkervalues.BlinkerPos.x += wagain + Lines[blinkervalues.num].innertexts[blinkervalues.blinkersurfacelinepartindex].settings.CharSpacing;
                }
            }
            else if (value==-1){
                if (blinkervalues.linepartindex==0){
                    if (blinkervalues.blinkersurfacelinepartindex==0){
                        blinkervalues.BlinkerPos.x = Lines[blinkervalues.num].innertexts[blinkervalues.blinkersurfacelinepartindex].settings.Dimensions.x;
                        blinkervalues.linepartindex = -1;
                    }
                    else{
                        std::string justtemp;
                        int wagain,hagain;
                        blinkervalues.blinkersurfacelinepartindex-=1;
                        blinkervalues.linepartindex = Lines[blinkervalues.num].innertexts[blinkervalues.blinkersurfacelinepartindex].content.size()-1;
                        justtemp.push_back(Lines[blinkervalues.num].innertexts[blinkervalues.blinkersurfacelinepartindex+1].content[0]);
                        TTF_SizeUTF8(Lines[blinkervalues.num].innertexts[blinkervalues.blinkersurfacelinepartindex-1].settings.gFont,justtemp.c_str(),&wagain,&hagain);
                        blinkervalues.BlinkerPos.x -= ((Lines[blinkervalues.num].innertexts[blinkervalues.blinkersurfacelinepartindex+1].settings.CharSpacing/2) + wagain + (Lines[blinkervalues.num].innertexts[blinkervalues.blinkersurfacelinepartindex].settings.CharSpacing/2));
                    }
                }
                else if (blinkervalues.linepartindex==-1){
                    if (blinkervalues.num!=0){
                        blinkervalues.num--;
                        blinkervalues.blinkersurfacelinepartindex = Lines[blinkervalues.num].innertexts.size()-1;
                        blinkervalues.linepartindex = Lines[blinkervalues.num].innertexts[blinkervalues.blinkersurfacelinepartindex].content.size()-1;
                        blinkervalues.BlinkerPos.x = Lines[blinkervalues.num].innertexts[blinkervalues.blinkersurfacelinepartindex].settings.Dimensions.x + Lines[blinkervalues.num].innertexts[blinkervalues.blinkersurfacelinepartindex].settings.Dimensions.w - (Lines[blinkervalues.num].innertexts[blinkervalues.blinkersurfacelinepartindex].settings.CharSpacing/2);
                        blinkervalues.BlinkerPos.y = Lines[blinkervalues.num].innertexts[blinkervalues.blinkersurfacelinepartindex].settings.Dimensions.y;
                    }
                    else{
                        return;
                    }
                }
                else{
                    std::string justtemp="";
                    int wagaini,hagaini;
                    blinkervalues.linepartindex -=1;
                    justtemp.push_back(Lines[blinkervalues.num].innertexts[blinkervalues.blinkersurfacelinepartindex].content[blinkervalues.linepartindex + 1]);
                    TTF_SizeUTF8(Lines[blinkervalues.num].innertexts[blinkervalues.blinkersurfacelinepartindex].settings.gFont,justtemp.c_str(),&wagaini,&hagaini);
                    blinkervalues.BlinkerPos.x -= (wagaini + Lines[blinkervalues.num].innertexts[blinkervalues.blinkersurfacelinepartindex].settings.CharSpacing);
                }
            }
            TTF_SizeUTF8(WholeSettings.gFont,"|",&blinkervalues.BlinkerPos.w,&blinkervalues.BlinkerPos.h);
        }

        void HandleEvents(SDL_Event *e) {
            // Check for mouse clicks and handle accordingly
            if (e->type == SDL_MOUSEBUTTONDOWN && active) {
                if (e->button.button == SDL_BUTTON_LEFT) {
                    int temp_x, temp_y;
                    SDL_GetMouseState(&temp_x, &temp_y);
                    int line_no = 0;
                    for (textinputs temp : Lines) {
                        if (temp_x > temp.innertexts[0].settings.Dimensions.x && 
                            temp_y > temp.innertexts[0].settings.Dimensions.y &&
                            temp_x < (temp.innertexts[0].settings.Dimensions.x + WholeSettings.Dimensions.w) &&
                            temp_y < (temp.maxHeight + temp.maxLineSpacing / 2)) {
                            
                            SDL_SetCursor(hovercursor);
                            int star = 0;
                            int linepartin = 0;

                            for (LinePart temp2 : temp.innertexts) {
                                int indexofchar = 0;
                                if (temp2.content.empty() && temp.innertexts.size() == 1) {
                                    int just_there_w, just_there_h;
                                    std::string man2 = "|";
                                    blinker = true;
                                    blinkervalues.num = line_no;
                                    blinkervalues.blinkersurfacelinepartindex = 0;
                                    blinkervalues.linepartindex = -1;
                                    TTF_SizeUTF8(temp2.settings.gFont, man2.c_str(), &just_there_w, &just_there_h);
                                    blinkervalues.BlinkerPos = {temp2.settings.Dimensions.x, temp2.settings.Dimensions.y, just_there_w, just_there_h};
                                    break;
                                } else {
                                    for (char b : temp2.content) {
                                        int just_there_w, just_there_h;
                                        std::string man(1, b);
                                        std::string man2 = "|";
                                        int star2 = (star == 0) ? star - temp2.settings.CharSpacing : star;
                                        TTF_SizeUTF8(temp2.settings.gFont, man.c_str(), &just_there_w, &just_there_h);

                                        if (temp_x < star2 + (just_there_w / 2) + temp2.settings.CharSpacing) {
                                            blinker = true;
                                            TTF_SizeUTF8(temp2.settings.gFont, man2.c_str(), &just_there_w, &just_there_h);
                                            blinkervalues.BlinkerPos = {star + (temp2.settings.CharSpacing / 2), temp.innertexts[0].settings.Dimensions.y, just_there_w, just_there_h};
                                            blinkervalues.num = line_no;
                                            blinkervalues.linepartindex = indexofchar;
                                            blinkervalues.blinkersurfacelinepartindex = linepartin;
                                            break;
                                        }
                                        star += just_there_w + temp2.settings.CharSpacing;
                                        indexofchar += 1;
                                    }
                                }
                                linepartin += 1;
                            }
                        }
                        line_no += 1;
                    }
                }
            }

            // Process text input (SDL_TEXTINPUT) if blinker is active
            if (e->type == SDL_TEXTINPUT && blinker) {
                putthetext(*e->text.text);
            }
            bool isPasting = false;

            if (e->type == SDL_KEYDOWN) {
                if (e->key.keysym.mod & KMOD_CTRL) {
                    if (e->key.keysym.sym == SDLK_v && !isPasting) {
                        isPasting = true; // Set the flag to true to prevent double pasting
                        if (SDL_HasClipboardText()) {
                            char* clipboardText = SDL_GetClipboardText();
                            if (clipboardText) {
                                for (char b : std::string(clipboardText)) {
                                    putthetext(b); // Your function to handle each character
                                }
                                SDL_free(clipboardText); // Free the clipboard text after use
                            }
                        }
                    }
                }
            } else if (e->type == SDL_KEYUP) {
                if (e->key.keysym.sym == SDLK_v) {
                    isPasting = false; // Reset the flag when the key is released
                }
            }


            // Process specific keys only for SDL_KEYDOWN
            if (e->type == SDL_KEYDOWN && blinker) {
                switch (e->key.keysym.sym) {
                    case SDLK_LEFT:
                        move_blinker(-1);
                        break;
                    case SDLK_RIGHT:
                        move_blinker(1);
                        break;
                    case SDLK_RETURN:
                        // Add any enter-key specific behavior here
                        break;
                    case SDLK_BACKSPACE:
                        removethetext();
                        break;
                    default:
                        // Ignore other keys
                        break;
                }
            }
        }



        void run(SDL_Event *e,SDL_Renderer* renderer){
            garbage_collector();
            HandleEvents(e);
            renderblinker(renderer);
            render(renderer); //this blits some surfaces to the main surface
        }
};

int main(int argc, char* argv[]) {    
    if (TTF_Init() == -1) {
        return -1;
    }
    
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        TTF_Quit();
        return -1;
    }
    SDL_Window* window = SDL_CreateWindow("text",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,640,480,SDL_WINDOW_SHOWN);

    SDL_Rect displayarea = {0, 0, 300, 300};
    TextInput basic(&displayarea,16);

    if (window == NULL) {
        SDL_Quit();
        TTF_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    bool quit = false;
    SDL_Event e;
    SDL_StartTextInput();
    int ggg = 0;

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)) {
                quit = true;
            }
            SDL_SetRenderDrawColor(renderer,0xFF,0xFF,0xFF,0xFF);
            SDL_RenderClear(renderer);
            basic.run(&e,renderer);
            SDL_RenderPresent(renderer);
            ggg++;
        }
    }

    SDL_StopTextInput();
    SDL_DestroyWindow(window);
    SDL_Quit();
    TTF_Quit();

    return 0;
}
