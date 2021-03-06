/************************************************************
  * @brief   按键驱动
	* @param   NULL
  * @return  NULL
  * @author  jiejie
  * @github  https://github.com/jiejieTop
  * @date    2018-xx-xx
  * @version v1.0
  * @note    button.c
  ***********************************************************/
#include "button.h"

/*******************************************************************
 *                          变量声明                               
 *******************************************************************/

static struct button* Head_Button = NULL;


/*******************************************************************
 *                         函数声明     
 *******************************************************************/
static char *StrnCopy(char *dst, const char *src, ubase_t n);
static void Print_Btn_Info(Button_t* btn);
static void Add_Button(Button_t* btn);


/************************************************************
  * @brief   按键创建
	* @param   name : 按键名称
	* @param   btn : 按键结构体
  * @param   read_btn_level : 按键电平读取函数，需要用户自己实现返回uint8_t类型的电平
  * @param   btn_trigger_level : 按键触发电平
  * @return  NULL
  * @author  jiejie
  * @github  https://github.com/jiejieTop
  * @date    2018-xx-xx
  * @version v1.0
  * @note    NULL
  ***********************************************************/
void Button_Create(const char *name,
                  Button_t *btn, 
                  uint8_t(*read_btn_level)(void),
                  uint8_t btn_trigger_level)
{
  if( btn == NULL)
  {
    PRINT_ERR("struct button is null!");
    ASSERT(ASSERT_ERR);
  }
  
  memset(btn, 0, sizeof(struct button));  //清除结构体信息，建议用户在之前清除
 
  StrnCopy(btn->Name, name, BTN_NAME_MAX); /* 创建按键名称 */
  
  
  btn->Button_State = NONE_TRIGGER;           //按键状态
  btn->Button_Last_State = NONE_TRIGGER;      //按键上一次状态
  btn->Button_Trigger_Event = NONE_TRIGGER;   //按键触发事件
  btn->Read_Button_Level = read_btn_level;    //按键读电平函数
  btn->Button_Trigger_Level = btn_trigger_level;  //按键触发电平
  btn->Button_Last_Level = btn->Read_Button_Level(); //按键当前电平
  btn->Debounce_Time = 0;
  
  PRINT_DEBUG("button create success!");
  
  Add_Button(btn);          //创建的时候添加到单链表中
  
  Print_Btn_Info(btn);     //打印信息
 
}

/************************************************************
  * @brief   按键触发事件与回调函数映射链接起来
	* @param   btn : 按键结构体
	* @param   btn_event : 按键触发事件
  * @param   btn_callback : 按键触发之后的回调处理函数。需要用户实现
  * @return  NULL
  * @author  jiejie
  * @github  https://github.com/jiejieTop
  * @date    2018-xx-xx
  * @version v1.0
  ***********************************************************/
void Button_Attach(Button_t *btn,Button_Event btn_event,Button_CallBack btn_callback)
{
  if( btn == NULL)
  {
    PRINT_ERR("struct button is null!");
    ASSERT(ASSERT_ERR);       //断言
  }
  
  if(BUTTON_ALL_RIGGER == btn_event)
  {
    for(uint8_t i = 0 ; i < number_of_event-1 ; i++)
      btn->CallBack_Function[i] = btn_callback; //按键事件触发的回调函数，用于处理按键事件
  }
  else
  {
    btn->CallBack_Function[btn_event] = btn_callback; //按键事件触发的回调函数，用于处理按键事件
  }
}

/************************************************************
  * @brief   删除一个已经创建的按键
	* @param   NULL
  * @return  NULL
  * @author  jiejie
  * @github  https://github.com/jiejieTop
  * @date    2018-xx-xx
  * @version v1.0
  * @note    NULL
  ***********************************************************/
void Button_Delete(Button_t *btn)
{
  struct button** curr;
  for(curr = &Head_Button; *curr;) 
  {
    struct button* entry = *curr;
    if (entry == btn) 
    {
      *curr = entry->Next;
    } 
    else
    {
      curr = &entry->Next;
    }
  }
}

/************************************************************
  * @brief   获取按键触发的事件
	* @param   NULL
  * @return  NULL
  * @author  jiejie
  * @github  https://github.com/jiejieTop
  * @date    2018-xx-xx
  * @version v1.0
  ***********************************************************/
uint8_t Get_Button_Event(Button_t *btn)
{
  return (uint8_t)(btn->Button_Trigger_Event);
}

/************************************************************
  * @brief   获取按键触发的事件
	* @param   NULL
  * @return  NULL
  * @author  jiejie
  * @github  https://github.com/jiejieTop
  * @date    2018-xx-xx
  * @version v1.0
  ***********************************************************/
uint8_t Get_Button_State(Button_t *btn)
{
  return (uint8_t)(btn->Button_State);
}

/************************************************************
  * @brief   按键周期处理函数
  * @param   btn:处理的按键
  * @return  NULL
  * @author  jiejie
  * @github  https://github.com/jiejieTop
  * @date    2018-xx-xx
  * @version v1.0
  * @note    必须以一定周期调用此函数，建议周期为20~50ms
  ***********************************************************/
void Button_Cycle_Process(Button_t *btn)
{
  uint8_t current_level = (uint8_t)btn->Read_Button_Level();//获取当前按键电平
  
//  PRINT_DEBUG("1:btn->Button_State = %d",btn->Button_State);
  
  if((current_level != btn->Button_Last_Level)&&(++(btn->Debounce_Time) >= BUTTON_DEBOUNCE_TIME)) //按键电平发生变化，消抖
  {
      btn->Button_Last_Level = current_level; //更新当前按键电平
      btn->Debounce_Time = 0;                 //确定了是按下
      
      //如果按键是没被按下的，改变按键状态为按下(首次按下)
      if(btn->Button_State == NONE_TRIGGER)
      {
          btn->Button_State = BUTTON_DOWM;
          PRINT_DEBUG("首次按下");
      }
      //释放按键
      else if(btn->Button_State == BUTTON_DOWM)
      {
        btn->Button_State = BUTTON_UP;
        PRINT_DEBUG("释放了按键");
      }
  }
  
  switch(btn->Button_State)
  {
    case BUTTON_DOWM :
    {
      if(btn->Button_Last_Level == btn->Button_Trigger_Level) //按键按下
      {
        btn->Button_Trigger_Event = BUTTON_DOWM;
        
        #if CONTINUOS_TRIGGER     //支持连续触发
        if(++(btn->Button_Cycle) >= BUTTON_CYCLE)
        {
          btn->Button_Cycle = 0;
          TRIGGER_CB(BUTTON_DOWM);    //连按
          PRINT_DEBUG("连按");
        }
        #endif
        
        if(++(btn->Long_Time) >= BUTTON_LONG_TIME)  //释放按键前更新触发事件为长按
        {
          btn->Button_Trigger_Event = BUTTON_LONG; 
          PRINT_DEBUG("长按");
        }
        
      }
      else    //如果是不支持连按的，检测释放按键
      {
        btn->Button_State = BUTTON_UP;
      }

      break;
    } 
    
    case BUTTON_UP :
    {
      if(btn->Button_Trigger_Event == BUTTON_DOWM)  //按下单击
      {
        // 双击
        if((btn->Timer_Count <= BUTTON_DOUBLE_TIME)&&(btn->Button_Last_State == BUTTON_DOWM))
        {
          TRIGGER_CB(BUTTON_DOUBLE);    
          PRINT_DEBUG("双击");
          btn->Button_State = NONE_TRIGGER;
          btn->Button_Last_State = NONE_TRIGGER;
        }
        else
        {
          btn->Timer_Count=0;
          btn->Long_Time = 0;   //检测长按失败，清0
          TRIGGER_CB(BUTTON_DOWM);    //单击
          btn->Button_State = NONE_TRIGGER;
          btn->Button_Last_State = BUTTON_DOWM;
        }
      }
      
      else if(btn->Button_Trigger_Event == BUTTON_LONG)
      {
        TRIGGER_CB(BUTTON_LONG);    //长按
        btn->Long_Time = 0;
        btn->Button_State = NONE_TRIGGER;
        btn->Button_Last_State = BUTTON_LONG;
      } 
      break;
    }
    
    case NONE_TRIGGER :
    {
      btn->Timer_Count++;     //时间记录
      break;
    }
    
    default :
      break;
  }
  
}
/************************************************************
  * @brief   遍历的方式扫描按键，不会丢失每个按键
	* @param   NULL
  * @return  NULL
  * @author  jiejie
  * @github  https://github.com/jiejieTop
  * @date    2018-xx-xx
  * @version v1.0
  * @note    此函数要周期调用，建议20-50ms调用一次
  ***********************************************************/
void Button_Process(void)
{
  struct button* pass_btn;
  for(pass_btn = Head_Button; pass_btn != NULL; pass_btn = pass_btn->Next)
  {
      Button_Cycle_Process(pass_btn);
  }
}

/************************************************************
  * @brief   遍历按键
	* @param   NULL
  * @return  NULL
  * @author  jiejie
  * @github  https://github.com/jiejieTop
  * @date    2018-xx-xx
  * @version v1.0
  * @note    NULL
  ***********************************************************/
void Search_Button(void)
{
  struct button* pass_btn;
  for(pass_btn = Head_Button; pass_btn != NULL; pass_btn = pass_btn->Next)
  {
    PRINT_INFO("button node have %s",pass_btn->Name);
  }
}



/**************************** 以下是内部调用函数 ********************/

/************************************************************
  * @brief   拷贝指定长度字符串
	* @param   NULL
  * @return  NULL
  * @author  jiejie
  * @github  https://github.com/jiejieTop
  * @date    2018-xx-xx
  * @version v1.0
  * @note    NULL
  ***********************************************************/
static char *StrnCopy(char *dst, const char *src, ubase_t n)
{
  if (n != 0)
  {
    char *d = dst;
    const char *s = src;
    do
    {
        if ((*d++ = *s++) == 0)
        {
            /* NUL pad the remaining n-1 bytes */
            while (--n != 0)
                *d++ = 0;
            break;
        }
    } while (--n != 0);
  }
  return (dst);
}

/************************************************************
  * @brief   打印按键相关信息
	* @param   NULL
  * @return  NULL
  * @author  jiejie
  * @github  https://github.com/jiejieTop
  * @date    2018-xx-xx
  * @version v1.0
  * @note    NULL
  ***********************************************************/
static void Print_Btn_Info(Button_t* btn)
{
  
  PRINT_INFO("button struct information:\n\
              btn->Name:%s \n\
              btn->Button_State:%d \n\
              btn->Button_Trigger_Event:%d \n\
              btn->Button_Trigger_Level:%d \n\
              btn->Button_Last_Level:%d \n\
              ",
              btn->Name,
              btn->Button_State,
              btn->Button_Trigger_Event,
              btn->Button_Trigger_Level,
              btn->Button_Last_Level);
  Search_Button();
}
/************************************************************
  * @brief   使用单链表将按键连接起来
	* @param   NULL
  * @return  NULL
  * @author  jiejie
  * @github  https://github.com/jiejieTop
  * @date    2018-xx-xx
  * @version v1.0
  * @note    NULL
  ***********************************************************/
static void Add_Button(Button_t* btn)
{
  struct button *pass_btn = Head_Button;
  
  while(pass_btn)
  {
    pass_btn = pass_btn->Next;
  }
  
  btn->Next = Head_Button;
  Head_Button = btn;
}





