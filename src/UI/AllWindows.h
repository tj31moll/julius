#ifndef UI_ALLWINDOWS_H
#define UI_ALLWINDOWS_H

void UI_MainMenu_drawBackground();
void UI_MainMenu_drawForeground();
void UI_MainMenu_handleMouse();

void UI_DifficultyOptions_drawForeground();
void UI_DifficultyOptions_handleMouse();

void UI_DisplayOptions_drawForeground();

void UI_SoundOptions_init();
void UI_SoundOptions_drawForeground();
void UI_SoundOptions_handleMouse();

void UI_SpeedOptions_init();
void UI_SpeedOptions_drawForeground();
void UI_SpeedOptions_handleMouse();

void UI_Advisors_setAdvisor(int advisor);
void UI_Advisors_init();
void UI_Advisors_drawBackground();
void UI_Advisors_drawForeground();
void UI_Advisors_handleMouse();

void UI_LaborPriorityDialog_drawBackground();
void UI_LaborPriorityDialog_drawForeground();
void UI_LaborPriorityDialog_handleMouse();

void UI_SetSalaryDialog_drawBackground();
void UI_SetSalaryDialog_drawForeground();
void UI_SetSalaryDialog_handleMouse();

void UI_DonateToCityDialog_init();
void UI_DonateToCityDialog_drawBackground();
void UI_DonateToCityDialog_drawForeground();
void UI_DonateToCityDialog_handleMouse();

void UI_SendGiftToCaesarDialog_init();
void UI_SendGiftToCaesarDialog_drawBackground();
void UI_SendGiftToCaesarDialog_drawForeground();
void UI_SendGiftToCaesarDialog_handleMouse();

void UI_TradePricesDialog_drawBackground();
void UI_TradePricesDialog_handleMouse();

void UI_ResourceSettingsDialog_drawBackground();
void UI_ResourceSettingsDialog_drawForeground();
void UI_ResourceSettingsDialog_handleMouse();

void UI_HoldFestivalDialog_drawBackground();
void UI_HoldFestivalDialog_drawForeground();
void UI_HoldFestivalDialog_handleMouse();

void UI_City_drawBackground();
void UI_City_drawForeground();
void UI_City_handleMouse();

void UI_Empire_drawBackground();
void UI_Empire_drawForeground();
void UI_Empire_handleMouse();

void UI_TradeOpenedDialog_drawBackground();
void UI_TradeOpenedDialog_drawForeground();
void UI_TradeOpenedDialog_handleMouse();

#endif
