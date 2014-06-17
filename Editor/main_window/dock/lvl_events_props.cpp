/*
 * Platformer Game Engine by Wohlstand, a free platform for game making
 * Copyright (c) 2014 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QMediaPlayer>

#include "../../ui_mainwindow.h"
#include "../../mainwindow.h"


#include "../../level_scene/item_bgo.h"
#include "../../level_scene/item_block.h"
#include "../../level_scene/item_npc.h"
#include "../../level_scene/item_water.h"

#include "../../level_scene/itemmsgbox.h"

#include "../../file_formats/file_formats.h"

static long currentEventArrayID=0;
static bool lockSetEventSettings=false;

QMediaPlayer playSnd;

void MainWindow::setEventsBox()
{
    int WinType = activeChildWindow();
    QListWidgetItem * item;

    ui->LVLEvents_List->clear();

    if (WinType==1)
    {
        foreach(LevelEvents event, activeLvlEditWin()->LvlData.events)
        {
            item = new QListWidgetItem;
            item->setText(event.name);
            //item->setFlags(Qt::ItemIsUserCheckable);

            item->setFlags(item->flags() | Qt::ItemIsEnabled | Qt::ItemIsSelectable);

            if((event.name!="Level - Start")&&(event.name!="P Switch - Start")&&(event.name!="P Switch - End"))
                item->setFlags(item->flags() | Qt::ItemIsEditable | Qt::ItemIsDragEnabled);

            //item->setCheckState( (layer.hidden) ? Qt::Unchecked: Qt::Checked );
            item->setData(3, QString::number( event.array_id ) );
            ui->LVLEvents_List->addItem( item );
        }

        on_LVLEvents_List_itemSelectionChanged();
    }
}

void MainWindow::EventListsSync()
{
    LvlItemPropsLock = true;
    int WinType = activeChildWindow();

    ui->PROPS_BlkEventDestroy->clear();
    ui->PROPS_BlkEventHited->clear();
    ui->PROPS_BlkEventLayerEmpty->clear();

    ui->PROPS_NpcEventActivate->clear();
    ui->PROPS_NpcEventDeath->clear();
    ui->PROPS_NpcEventTalk->clear();
    ui->PROPS_NpcEventEmptyLayer->clear();
    ui->LVLEvent_TriggerEvent->clear();

    QString noEvent = tr("[None]");
    ui->PROPS_BlkEventDestroy->addItem(noEvent);
    ui->PROPS_BlkEventHited->addItem(noEvent);
    ui->PROPS_BlkEventLayerEmpty->addItem(noEvent);

    ui->PROPS_NpcEventActivate->addItem(noEvent);
    ui->PROPS_NpcEventDeath->addItem(noEvent);
    ui->PROPS_NpcEventTalk->addItem(noEvent);
    ui->PROPS_NpcEventEmptyLayer->addItem(noEvent);
    ui->LVLEvent_TriggerEvent->addItem(noEvent);


    if (WinType==1)
    {
        foreach(LevelEvents event, activeLvlEditWin()->LvlData.events)
        {
            ui->PROPS_BlkEventDestroy->addItem(event.name);
            ui->PROPS_BlkEventHited->addItem(event.name);
            ui->PROPS_BlkEventLayerEmpty->addItem(event.name);

            ui->PROPS_NpcEventActivate->addItem(event.name);
            ui->PROPS_NpcEventDeath->addItem(event.name);
            ui->PROPS_NpcEventTalk->addItem(event.name);
            ui->PROPS_NpcEventEmptyLayer->addItem(event.name);
            ui->LVLEvent_TriggerEvent->addItem(event.name);
        }
    }
    LvlItemPropsLock = false;

}


void MainWindow::setSoundList()
{
    ui->LVLEvent_Cmn_PlaySnd->clear();
    ui->LVLEvent_Cmn_PlaySnd->addItem( tr("[Silence]"), "0" );

    if(configs.check()) return;

    foreach(obj_sound snd, configs.main_sound )
    {
        if(!snd.hidden)
            ui->LVLEvent_Cmn_PlaySnd->addItem(snd.name, QString::number(snd.id));
    }
}



void MainWindow::setEventData(long index)
{
    lockSetEventSettings=true;
    long cIndex;
    bool found=false;
    if(index==-2)
        {
        if(!ui->LVLEvents_List->selectedItems().isEmpty())
            cIndex = ui->LVLEvents_List->currentItem()->data(3).toInt();
        else
            cIndex = currentEventArrayID;
        }
    else
        cIndex = index;

    int WinType = activeChildWindow();
    if (WinType==1)
    {

        if( (activeLvlEditWin()->LvlData.events.size() > 0) && (cIndex >= 0))
        {
            leveledit * edit = activeLvlEditWin();
            currentEventArrayID=cIndex;
            foreach(LevelEvents event, edit->LvlData.events)
            {
                if(event.array_id == (unsigned int)index)
                {
                    currentEventArrayID=event.array_id;

                    //Enable controls
                    ui->LVLEvents_Settings->setEnabled(true);
                    ui->LVLEvent_AutoStart->setEnabled(true);

                    //Set controls data
                    ui->LVLEvent_AutoStart->setChecked( event.autostart );

                    //Layers visibly - layerList
                    ui->LVLEvent_disableSmokeEffect->setChecked( event.nosmoke );
                    eventLayerVisiblySyncList();

                    // Layer Movement
                    ui->LVLEvent_LayerMov_List->setCurrentIndex(0);
                    for(int i=0; i< ui->LVLEvent_LayerMov_List->count(); i++)
                    {
                        if(ui->LVLEvent_LayerMov_List->itemText(i)==event.movelayer)
                        {
                            ui->LVLEvent_LayerMov_List->setCurrentIndex(i);
                            break;
                        }
                    }
                    ui->LVLEvent_LayerMov_spX->setValue(event.layer_speed_x);
                    ui->LVLEvent_LayerMov_spY->setValue(event.layer_speed_y);

                    //Scroll section / Move Camera
                    ui->LVLEvent_Scroll_Sct->setMaximum( edit->LvlData.sections.size() );
                    ui->LVLEvent_Scroll_Sct->setValue( event.scroll_section+1 );
                    ui->LVLEvent_Scroll_spX->setValue( event.move_camera_x );
                    ui->LVLEvent_Scroll_spY->setValue( event.move_camera_y );

                    //Section Settings
                    ui->LVLEvent_Sct_list->clear();
                    for(int i=0; i<edit->LvlData.sections.size(); i++)
                        ui->LVLEvent_Sct_list->addItem(tr("Section")+QString(" ")+QString::number(i+1), QString::number(i));

                    if(ui->LVLEvent_Sct_list->currentData().toInt()<event.sets.size())
                    {
                        ui->LVLEvent_SctSize_left->setText("");
                        ui->LVLEvent_SctSize_top->setText("");
                        ui->LVLEvent_SctSize_bottom->setText("");
                        ui->LVLEvent_SctSize_right->setText("");
                        ui->LVLEvent_SctSize_left->setEnabled(false);
                        ui->LVLEvent_SctSize_top->setEnabled(false);
                        ui->LVLEvent_SctSize_bottom->setEnabled(false);
                        ui->LVLEvent_SctSize_right->setEnabled(false);
                    switch(event.sets[ui->LVLEvent_Sct_list->currentData().toInt()].position_left)
                        {
                        case -1:
                            ui->LVLEvent_SctSize_none->setChecked(true);
                            break;
                        case -2:
                            ui->LVLEvent_SctSize_reset->setChecked(true);
                            break;
                        default:
                            ui->LVLEvent_SctSize_define->setChecked(true);
                            ui->LVLEvent_SctSize_left->setText(QString::number(event.sets[ui->LVLEvent_Sct_list->currentData().toInt()].position_left));
                            ui->LVLEvent_SctSize_top->setText(QString::number(event.sets[ui->LVLEvent_Sct_list->currentData().toInt()].position_top));
                            ui->LVLEvent_SctSize_bottom->setText(QString::number(event.sets[ui->LVLEvent_Sct_list->currentData().toInt()].position_bottom));
                            ui->LVLEvent_SctSize_right->setText(QString::number(event.sets[ui->LVLEvent_Sct_list->currentData().toInt()].position_right));
                            ui->LVLEvent_SctSize_left->setEnabled(true);
                            ui->LVLEvent_SctSize_top->setEnabled(true);
                            ui->LVLEvent_SctSize_bottom->setEnabled(true);
                            ui->LVLEvent_SctSize_right->setEnabled(true);
                            break;
                        }

                    ui->LVLEvent_SctMus_List->setEnabled(false);
                    switch(event.sets[ui->LVLEvent_Sct_list->currentData().toInt()].music_id)
                        {
                        case -1:
                            ui->LVLEvent_SctMus_none->setChecked(true);
                            break;
                        case -2:
                            ui->LVLEvent_SctMus_reset->setChecked(true);
                            break;
                        default:
                            ui->LVLEvent_SctMus_define->setChecked(true);
                            ui->LVLEvent_SctMus_List->setEnabled(true);
                            break;
                        }

                    ui->LVLEvent_SctBg_List->setEnabled(false);
                    switch(event.sets[ui->LVLEvent_Sct_list->currentData().toInt()].background_id)
                        {
                        case -1:
                            ui->LVLEvent_SctBg_none->setChecked(true);
                            break;
                        case -2:
                            ui->LVLEvent_SctBg_reset->setChecked(true);
                            break;
                        default:
                            ui->LVLEvent_SctBg_define->setChecked(true);
                            ui->LVLEvent_SctBg_List->setEnabled(true);
                            break;
                        }


                    }

                    //Common settings
                    QString evnmsg = (event.msg.isEmpty() ? tr("[none]") : event.msg);
                    if(evnmsg.size()>20)
                    {
                        evnmsg.resize(18);
                        evnmsg.push_back("...");
                    }
                    ui->LVLEvent_Cmn_Msg->setText( evnmsg );

                    ui->LVLEvent_Cmn_PlaySnd->setCurrentIndex(0);
                    for(int i=0; i<ui->LVLEvent_Cmn_PlaySnd->count(); i++)
                    {
                        if(ui->LVLEvent_Cmn_PlaySnd->itemData(i).toInt() == event.sound_id)
                        {
                            ui->LVLEvent_Cmn_PlaySnd->setCurrentIndex(i);
                            break;
                        }
                    }

                    if(event.end_game<ui->LVLEvent_Cmn_EndGame->count())
                        ui->LVLEvent_Cmn_EndGame->setCurrentIndex(event.end_game);


                    //Player Control key hold
                    ui->LVLEvent_Key_AltJump->setChecked(event.altjump);
                    ui->LVLEvent_Key_AltRun->setChecked(event.altrun);
                    ui->LVLEvent_Key_Jump->setChecked(event.jump);
                    ui->LVLEvent_Key_Run->setChecked(event.run);
                    ui->LVLEvent_Key_Start->setChecked(event.start);
                    ui->LVLEvent_Key_Drop->setChecked(event.drop);
                    ui->LVLEvent_Key_Left->setChecked(event.left);
                    ui->LVLEvent_Key_Right->setChecked(event.right);
                    ui->LVLEvent_Key_Up->setChecked(event.up);
                    ui->LVLEvent_Key_Down->setChecked(event.down);

                    //Trigger Event
                    ui->LVLEvent_TriggerEvent->setCurrentIndex(0);
                    for(int i=0; i<ui->LVLEvent_TriggerEvent->count(); i++)
                    {
                        if(ui->LVLEvent_TriggerEvent->itemText(i)== event.trigger)
                        {
                            ui->LVLEvent_TriggerEvent->setCurrentIndex(i);
                            break;
                        }
                    }
                    ui->LVLEvent_TriggerDelay->setValue( qreal(event.trigger_timer)/10 );

                    found=true;
                    break;
                }
            }
        }

        if(!found)
        {
            ui->LVLEvents_Settings->setEnabled(false);
            ui->LVLEvent_AutoStart->setEnabled(false);
        }

        currentEventArrayID=cIndex;
    }
    lockSetEventSettings=false;

}


void MainWindow::eventLayerVisiblySyncList()
{
    int WinType = activeChildWindow();

    if (WinType==1)
    {
        leveledit * edit = activeLvlEditWin();
        long i = getEventArrayIndex();
        if(i<0) return;

        LevelEvents event = edit->LvlData.events[i];

        ui->LVLEvents_layerList->clear();
        ui->LVLEvent_Layer_HideList->clear();
        ui->LVLEvent_Layer_ShowList->clear();
        ui->LVLEvent_Layer_ToggleList->clear();

        QListWidgetItem * item;
        //Total layers list
        foreach(LevelLayers layer, edit->LvlData.layers)
        {
            item = new QListWidgetItem;
            item->setText(layer.name);
            item->setFlags(item->flags() | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            item->setData(3, QString::number( layer.array_id ) );
            ui->LVLEvents_layerList->addItem( item );
        }

        //Hidden layers
        foreach(QString layer, event.layers_hide)
        {
            item = new QListWidgetItem;
            item->setText(layer);
            item->setFlags(item->flags() | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            QList<QListWidgetItem *> items =
                  ui->LVLEvents_layerList->findItems(QString("*"), Qt::MatchWrap | Qt::MatchWildcard);
            foreach(QListWidgetItem *item, items)
            {
                if(item->text()==layer)
                { delete item; break;}
            }
            ui->LVLEvent_Layer_HideList->addItem( item );
        }

        //Showed layers
        foreach(QString layer, event.layers_show)
        {
            item = new QListWidgetItem;
            item->setText(layer);
            item->setFlags(item->flags() | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            QList<QListWidgetItem *> items =
                  ui->LVLEvents_layerList->findItems(QString("*"), Qt::MatchWrap | Qt::MatchWildcard);
            foreach(QListWidgetItem *item, items)
            {
                if(item->text()==layer)
                { delete item; break;}
            }
            ui->LVLEvent_Layer_ShowList->addItem( item );
        }

        //Toggeled layers
        foreach(QString layer, event.layers_toggle)
        {
            item = new QListWidgetItem;
            item->setText(layer);
            item->setFlags(item->flags() | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            QList<QListWidgetItem *> items =
                  ui->LVLEvents_layerList->findItems(QString("*"), Qt::MatchWrap | Qt::MatchWildcard);
            foreach(QListWidgetItem *item, items)
            {
                if(item->text()==layer)
                { delete item; break;}
            }
            ui->LVLEvent_Layer_ToggleList->addItem( item );
        }
    }
}

void MainWindow::on_LVLEvents_List_itemSelectionChanged()
{
    if(ui->LVLEvents_List->selectedItems().isEmpty())
    {
        setEventData(-1);
    }
    else
        setEventData(ui->LVLEvents_List->currentItem()->data(3).toInt());

}


void MainWindow::on_LVLEvents_List_itemChanged(QListWidgetItem *item)
{
    int WinType = activeChildWindow();

    if (WinType==1)
    {
        if(item->data(3).toString()=="NewEvent")
        {
            bool AlreadyExist=false;
            foreach(LevelEvents event, activeLvlEditWin()->LvlData.events)
            {
                if( event.name==item->text() )
                {
                    AlreadyExist=true;
                    break;
                }
            }

            if(AlreadyExist)
            {
                delete item;
                return;
            }
            else
            {
                LevelEvents NewEvent = FileFormats::dummyLvlEvent();
                NewEvent.name = item->text();
                activeLvlEditWin()->LvlData.events_array_id++;
                NewEvent.array_id = activeLvlEditWin()->LvlData.events_array_id;

                item->setData(3, QString::number(NewEvent.array_id));

                activeLvlEditWin()->LvlData.events.push_back(NewEvent);
                activeLvlEditWin()->LvlData.modified=true;
            }

        }//if(item->data(3).toString()=="NewEvent")
        else
        {
            QString eventName = item->text();
            QString oldEventName = item->text();

            ModifyEventItem(item, oldEventName, eventName);
            activeLvlEditWin()->LvlData.modified=true;
        }

    }//if WinType==1
    EventListsSync();
}

long MainWindow::getEventArrayIndex()
{
    if(activeChildWindow()!=1) return -2;

    leveledit * edit = activeLvlEditWin();
    bool found=false;
    long i;
    for(i=0; i< edit->LvlData.events.size();i++)
    {
        if((unsigned long)currentEventArrayID==edit->LvlData.events[i].array_id)
        {
            found=true;
            break;
        }
    }
    if(!found)
        return -1;
    else
        return i;
}


void MainWindow::AddNewEvent(QString eventName, bool setEdited)
{
    QListWidgetItem * item;
    item = new QListWidgetItem;
    item->setText(eventName);
    item->setFlags(Qt::ItemIsEditable);
    item->setFlags(item->flags() | Qt::ItemIsEnabled);
    item->setFlags(item->flags() | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsSelectable);
    item->setData(3, QString("NewEvent") );
    ui->LVLEvents_List->addItem( item );

    if(setEdited)
    {
        ui->LVLEvents_List->setFocus();
        ui->LVLEvents_List->editItem( item );
    }
    else
    {
        bool AlreadyExist=false;
        foreach(LevelEvents event, activeLvlEditWin()->LvlData.events)
        {
            if( event.name==item->text() )
            {
                AlreadyExist=true;
                break;
            }
        }

        if(AlreadyExist)
        {
            delete item;
            return;
        }
        else
        {
            LevelEvents NewEvent = FileFormats::dummyLvlEvent();
            NewEvent.name = item->text();
            activeLvlEditWin()->LvlData.events_array_id++;
            NewEvent.array_id = activeLvlEditWin()->LvlData.events_array_id;
            item->setData(3, QString::number(NewEvent.array_id));

            activeLvlEditWin()->LvlData.events.push_back(NewEvent);
            activeLvlEditWin()->LvlData.modified=true;
        }
    }
    EventListsSync();  //Sync comboboxes in properties
}

void MainWindow::ModifyEventItem(QListWidgetItem *item, QString oldEventName, QString newEventName)
{
    //Find layer enrty in array and apply settings
    for(int i=0; i < activeLvlEditWin()->LvlData.events.size(); i++)
    {
        if( activeLvlEditWin()->LvlData.events[i].array_id==(unsigned int)item->data(3).toInt() )
        {
            oldEventName = activeLvlEditWin()->LvlData.events[i].name;
            activeLvlEditWin()->LvlData.events[i].name = newEventName;
            break;
        }
    }
    EventListsSync();  //Sync comboboxes in properties
}

void MainWindow::on_LVLEvents_add_clicked()
{
    AddNewEvent(tr("New Event %1").arg( ui->LVLEvents_List->count()+1 ), true);

}

void MainWindow::on_LVLEvents_del_clicked()
{
    if(ui->LVLEvents_List->selectedItems().isEmpty()) return;


}

void MainWindow::on_LVLEvents_duplicate_clicked()
{

}


void MainWindow::on_LVLEvent_AutoStart_clicked(bool checked)
{
    if(lockSetEventSettings) return;

    int WinType = activeChildWindow();

    if (WinType==1)
    {
        leveledit * edit = activeLvlEditWin();
        long i = getEventArrayIndex();
        if(i<0) return;

        edit->LvlData.events[i].autostart = checked;
        edit->LvlData.modified=true;
    }
}







void MainWindow::on_LVLEvent_disableSmokeEffect_clicked(bool checked)
{
    if(lockSetEventSettings) return;

    int WinType = activeChildWindow();

    if (WinType==1)
    {
        leveledit * edit = activeLvlEditWin();
        long i = getEventArrayIndex();
        if(i<0) return;

        edit->LvlData.events[i].nosmoke = checked;
        edit->LvlData.modified=true;
    }

}




void MainWindow::on_LVLEvent_Layer_HideAdd_clicked()
{
    if(lockSetEventSettings) return;

    int WinType = activeChildWindow();

    if (WinType==1)
    {
        leveledit * edit = activeLvlEditWin();
        long i = getEventArrayIndex();
        if(i<0) return;

        if(!ui->LVLEvents_layerList->selectedItems().isEmpty())
        {
            edit->LvlData.events[i].layers_hide.push_back(ui->LVLEvents_layerList->currentItem()->text());
            edit->LvlData.modified=true;
            eventLayerVisiblySyncList();
        }
    }

}

void MainWindow::on_LVLEvent_Layer_HideDel_clicked()
{
    if(lockSetEventSettings) return;

    int WinType = activeChildWindow();

    if (WinType==1)
    {
        leveledit * edit = activeLvlEditWin();
        long i = getEventArrayIndex();
        if(i<0) return;

        if(!ui->LVLEvent_Layer_HideList->selectedItems().isEmpty())
        {
            for(int j=0; j<edit->LvlData.events[i].layers_hide.size(); j++)
            {
            if(edit->LvlData.events[i].layers_hide[j]==ui->LVLEvent_Layer_HideList->currentItem()->text())
                {
                    edit->LvlData.events[i].layers_hide.removeAt(j);
                    edit->LvlData.modified=true;
                    break;
                }
            }
            eventLayerVisiblySyncList();
        }
    }
}

void MainWindow::on_LVLEvent_Layer_ShowAdd_clicked()
{
    if(lockSetEventSettings) return;

    int WinType = activeChildWindow();

    if (WinType==1)
    {
        leveledit * edit = activeLvlEditWin();
        long i = getEventArrayIndex();
        if(i<0) return;

        if(!ui->LVLEvents_layerList->selectedItems().isEmpty())
        {
            edit->LvlData.events[i].layers_show.push_back(ui->LVLEvents_layerList->currentItem()->text());
            edit->LvlData.modified=true;
            eventLayerVisiblySyncList();
        }
    }
}

void MainWindow::on_LVLEvent_Layer_ShowDel_clicked()
{
    if(lockSetEventSettings) return;

    int WinType = activeChildWindow();

    if (WinType==1)
    {
        leveledit * edit = activeLvlEditWin();
        long i = getEventArrayIndex();
        if(i<0) return;

        if(!ui->LVLEvent_Layer_ShowList->selectedItems().isEmpty())
        {
            for(int j=0; j<edit->LvlData.events[i].layers_show.size(); j++)
            {
            if(edit->LvlData.events[i].layers_show[j]==ui->LVLEvent_Layer_ShowList->currentItem()->text())
                {
                edit->LvlData.events[i].layers_show.removeAt(j);
                edit->LvlData.modified=true;
                break;
                }
            }
            eventLayerVisiblySyncList();
        }
    }

}

void MainWindow::on_LVLEvent_Layer_TogAdd_clicked()
{
    if(lockSetEventSettings) return;

    int WinType = activeChildWindow();

    if (WinType==1)
    {
        leveledit * edit = activeLvlEditWin();
        long i = getEventArrayIndex();
        if(i<0) return;

        if(!ui->LVLEvents_layerList->selectedItems().isEmpty())
        {
            edit->LvlData.events[i].layers_toggle.push_back(ui->LVLEvents_layerList->currentItem()->text());
            edit->LvlData.modified=true;
            eventLayerVisiblySyncList();
        }
    }
}

void MainWindow::on_LVLEvent_Layer_TogDel_clicked()
{
    if(lockSetEventSettings) return;

    int WinType = activeChildWindow();

    if (WinType==1)
    {
        leveledit * edit = activeLvlEditWin();
        long i = getEventArrayIndex();
        if(i<0) return;

        if(!ui->LVLEvent_Layer_ToggleList->selectedItems().isEmpty())
        {
            for(int j=0; j<edit->LvlData.events[i].layers_toggle.size(); j++)
            {
            if(edit->LvlData.events[i].layers_toggle[j]==ui->LVLEvent_Layer_ToggleList->currentItem()->text())
                {
                    edit->LvlData.events[i].layers_toggle.removeAt(j);
                    edit->LvlData.modified=true;
                    break;
                }
            }
            eventLayerVisiblySyncList();
        }
    }
}






void MainWindow::on_LVLEvent_LayerMov_List_currentIndexChanged(int index)
{
    if(lockSetEventSettings) return;

    if(index<0) return;

    int WinType = activeChildWindow();

    if (WinType==1)
    {
        leveledit * edit = activeLvlEditWin();
        long i = getEventArrayIndex();
        if(i<0) return;

        edit->LvlData.events[i].movelayer = ((index<=0)?"":ui->LVLEvent_LayerMov_List->currentText());
        edit->LvlData.modified=true;
    }
}

void MainWindow::on_LVLEvent_LayerMov_spX_valueChanged(double arg1)
{
    if(lockSetEventSettings) return;

    int WinType = activeChildWindow();

    if (WinType==1)
    {
        leveledit * edit = activeLvlEditWin();
        long i = getEventArrayIndex();
        if(i<0) return;

        edit->LvlData.events[i].layer_speed_x = arg1;
        edit->LvlData.modified=true;
    }

}

void MainWindow::on_LVLEvent_LayerMov_spY_valueChanged(double arg1)
{
    if(lockSetEventSettings) return;

    int WinType = activeChildWindow();

    if (WinType==1)
    {
        leveledit * edit = activeLvlEditWin();
        long i = getEventArrayIndex();
        if(i<0) return;

        edit->LvlData.events[i].layer_speed_y = arg1;
        edit->LvlData.modified=true;
    }

}







void MainWindow::on_LVLEvent_Scroll_Sct_valueChanged(int arg1)
{
    if(lockSetEventSettings) return;

    int WinType = activeChildWindow();

    if (WinType==1)
    {
        leveledit * edit = activeLvlEditWin();
        long i = getEventArrayIndex();
        if(i<0) return;

        edit->LvlData.events[i].scroll_section = arg1-1;
        edit->LvlData.modified=true;
    }
}

void MainWindow::on_LVLEvent_Scroll_spX_valueChanged(double arg1)
{
    if(lockSetEventSettings) return;

    int WinType = activeChildWindow();

    if (WinType==1)
    {
        leveledit * edit = activeLvlEditWin();
        long i = getEventArrayIndex();
        if(i<0) return;

        edit->LvlData.events[i].move_camera_x = arg1;
        edit->LvlData.modified=true;
    }

}

void MainWindow::on_LVLEvent_Scroll_spY_valueChanged(double arg1)
{
    if(lockSetEventSettings) return;

    int WinType = activeChildWindow();

    if (WinType==1)
    {
        leveledit * edit = activeLvlEditWin();
        long i = getEventArrayIndex();
        if(i<0) return;

        edit->LvlData.events[i].move_camera_y = arg1;
        edit->LvlData.modified=true;
    }
}




void MainWindow::on_LVLEvent_Sct_list_currentIndexChanged(int index)
{
    if(lockSetEventSettings) return;
}

void MainWindow::on_LVLEvent_SctSize_none_clicked()
{

}

void MainWindow::on_LVLEvent_SctSize_reset_clicked()
{

}

void MainWindow::on_LVLEvent_SctSize_define_clicked()
{

}

void MainWindow::on_LVLEvent_SctSize_left_textEdited(const QString &arg1)
{
    if(lockSetEventSettings) return;
}

void MainWindow::on_LVLEvent_SctSize_top_textEdited(const QString &arg1)
{
    if(lockSetEventSettings) return;
}

void MainWindow::on_LVLEvent_SctSize_bottom_textEdited(const QString &arg1)
{
    if(lockSetEventSettings) return;
}

void MainWindow::on_LVLEvent_SctSize_right_textEdited(const QString &arg1)
{
    if(lockSetEventSettings) return;
}

void MainWindow::on_LVLEvent_SctSize_Set_clicked()
{

}

void MainWindow::on_LVLEvent_SctMus_none_clicked()
{

}

void MainWindow::on_LVLEvent_SctMus_reset_clicked()
{

}

void MainWindow::on_LVLEvent_SctMus_define_clicked()
{

}

void MainWindow::on_LVLEvent_SctMus_List_currentIndexChanged(int index)
{
    if(lockSetEventSettings) return;
    if(index<0) return;
}

void MainWindow::on_LVLEvent_SctBg_none_clicked()
{

}

void MainWindow::on_LVLEvent_SctBg_reset_clicked()
{

}

void MainWindow::on_LVLEvent_SctBg_define_clicked()
{

}

void MainWindow::on_LVLEvent_SctBg_List_currentIndexChanged(int index)
{
    if(lockSetEventSettings) return;
    if(index<0) return;
}





void MainWindow::on_LVLEvent_Cmn_Msg_clicked()
{
    if(currentEventArrayID<0) return;

    int WinType = activeChildWindow();

    if (WinType==1)
    {
        leveledit * edit = activeLvlEditWin();
        long i = getEventArrayIndex();
        if(i<0) return;

        ItemMsgBox * msgBox = new ItemMsgBox(edit->LvlData.events[i].msg,
                tr("Please, enter message\nMessage limits: max line lenth is 28 characters"));
        msgBox->setWindowFlags (Qt::Window | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
        msgBox->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, msgBox->size(), qApp->desktop()->availableGeometry()));
        if(msgBox->exec()==QDialog::Accepted)
        {
            edit->LvlData.events[i].msg = msgBox->currentText;
            QString evnmsg = (edit->LvlData.events[i].msg.isEmpty() ? tr("[none]") : edit->LvlData.events[i].msg);
            if(evnmsg.size()>20)
            {
                evnmsg.resize(18);
                evnmsg.push_back("...");
            }
            ui->LVLEvent_Cmn_Msg->setText( evnmsg );
            edit->LvlData.modified=true;
        }

    }
}


void MainWindow::on_LVLEvent_Cmn_PlaySnd_currentIndexChanged(int index)
{
    if(lockSetEventSettings) return;
    if(index<0) return;

    int WinType = activeChildWindow();

    if (WinType==1)
    {
        leveledit * edit = activeLvlEditWin();
        long i = getEventArrayIndex();
        if(i<0) return;

        edit->LvlData.events[i].sound_id = ui->LVLEvent_Cmn_PlaySnd->currentData().toInt();
        edit->LvlData.modified=true;
    }
}

void MainWindow::on_LVLEvent_playSnd_clicked()
{
    if(ui->LVLEvent_Cmn_PlaySnd->currentData().toInt()==0) return;

    QString sndPath = configs.dirs.sounds;
    long i;
    bool found=false;
    for(i=0; i<configs.main_sound.size(); i++)
    {
        if((unsigned int)ui->LVLEvent_Cmn_PlaySnd->currentData().toInt()==configs.main_sound[i].id)
        {
            found=true;
            sndPath += configs.main_sound[i].file;
            break;
        }
    }

    WriteToLog(QtDebugMsg, QString("Test Sound -> path-1 %1").arg(sndPath));

    if(!found) return;
    if(!QFileInfo::exists(sndPath)) return;

    playSnd.setMedia(QMediaContent(QUrl(sndPath)));
    playSnd.setVolume(100);
    playSnd.play();
}



void MainWindow::on_LVLEvent_Cmn_EndGame_currentIndexChanged(int index)
{
    if(lockSetEventSettings) return;
    if(index<0) return;

    int WinType = activeChildWindow();

    if (WinType==1)
    {
        leveledit * edit = activeLvlEditWin();
        long i = getEventArrayIndex();
        if(i<0) return;

        edit->LvlData.events[i].end_game = ui->LVLEvent_Cmn_EndGame->currentIndex();
        edit->LvlData.modified=true;
    }

}







void MainWindow::on_LVLEvent_Key_Up_clicked(bool checked)
{

    if(lockSetEventSettings) return;

    int WinType = activeChildWindow();

    if (WinType==1)
    {
        leveledit * edit = activeLvlEditWin();
        long i = getEventArrayIndex();
        if(i<0) return;

        edit->LvlData.events[i].up = checked;
        edit->LvlData.modified=true;
    }

}

void MainWindow::on_LVLEvent_Key_Down_clicked(bool checked)
{
    if(lockSetEventSettings) return;

    int WinType = activeChildWindow();

    if (WinType==1)
    {
        leveledit * edit = activeLvlEditWin();
        long i = getEventArrayIndex();
        if(i<0) return;

        edit->LvlData.events[i].down = checked;
        edit->LvlData.modified=true;
    }
}

void MainWindow::on_LVLEvent_Key_Left_clicked(bool checked)
{
    if(lockSetEventSettings) return;

    int WinType = activeChildWindow();

    if (WinType==1)
    {
        leveledit * edit = activeLvlEditWin();
        long i = getEventArrayIndex();
        if(i<0) return;

        edit->LvlData.events[i].left = checked;
        edit->LvlData.modified=true;
    }
}

void MainWindow::on_LVLEvent_Key_Right_clicked(bool checked)
{
    if(lockSetEventSettings) return;

    int WinType = activeChildWindow();

    if (WinType==1)
    {
        leveledit * edit = activeLvlEditWin();
        long i = getEventArrayIndex();
        if(i<0) return;

        edit->LvlData.events[i].right = checked;
        edit->LvlData.modified=true;
    }
}

void MainWindow::on_LVLEvent_Key_Run_clicked(bool checked)
{
    if(lockSetEventSettings) return;

    int WinType = activeChildWindow();

    if (WinType==1)
    {
        leveledit * edit = activeLvlEditWin();
        long i = getEventArrayIndex();
        if(i<0) return;

        edit->LvlData.events[i].run = checked;
        edit->LvlData.modified=true;
    }
}

void MainWindow::on_LVLEvent_Key_AltRun_clicked(bool checked)
{
    if(lockSetEventSettings) return;

    int WinType = activeChildWindow();

    if (WinType==1)
    {
        leveledit * edit = activeLvlEditWin();
        long i = getEventArrayIndex();
        if(i<0) return;

        edit->LvlData.events[i].altrun = checked;
        edit->LvlData.modified=true;
    }
}

void MainWindow::on_LVLEvent_Key_Jump_clicked(bool checked)
{
    if(lockSetEventSettings) return;

    int WinType = activeChildWindow();

    if (WinType==1)
    {
        leveledit * edit = activeLvlEditWin();
        long i = getEventArrayIndex();
        if(i<0) return;

        edit->LvlData.events[i].jump = checked;
        edit->LvlData.modified=true;
    }
}

void MainWindow::on_LVLEvent_Key_AltJump_clicked(bool checked)
{
    if(lockSetEventSettings) return;

    int WinType = activeChildWindow();

    if (WinType==1)
    {
        leveledit * edit = activeLvlEditWin();
        long i = getEventArrayIndex();
        if(i<0) return;

        edit->LvlData.events[i].altjump = checked;
        edit->LvlData.modified=true;
    }
}

void MainWindow::on_LVLEvent_Key_Drop_clicked(bool checked)
{
    if(lockSetEventSettings) return;

    int WinType = activeChildWindow();

    if (WinType==1)
    {
        leveledit * edit = activeLvlEditWin();
        long i = getEventArrayIndex();
        if(i<0) return;

        edit->LvlData.events[i].drop = checked;
        edit->LvlData.modified=true;
    }
}

void MainWindow::on_LVLEvent_Key_Start_clicked(bool checked)
{
    if(lockSetEventSettings) return;

    int WinType = activeChildWindow();

    if (WinType==1)
    {
        leveledit * edit = activeLvlEditWin();
        long i = getEventArrayIndex();
        if(i<0) return;

        edit->LvlData.events[i].start = checked;
        edit->LvlData.modified=true;
    }
}




void MainWindow::on_LVLEvent_TriggerEvent_currentIndexChanged(int index)
{
    if(lockSetEventSettings) return;
    if(index<0) return;

    int WinType = activeChildWindow();

    if (WinType==1)
    {
        leveledit * edit = activeLvlEditWin();
        long i = getEventArrayIndex();
        if(i<0) return;

        edit->LvlData.events[i].trigger = ui->LVLEvent_TriggerEvent->currentText();
        edit->LvlData.modified=true;
    }

}

void MainWindow::on_LVLEvent_TriggerDelay_valueChanged(double arg1)
{
    if(lockSetEventSettings) return;

    int WinType = activeChildWindow();

    if (WinType==1)
    {
        leveledit * edit = activeLvlEditWin();
        long i = getEventArrayIndex();
        if(i<0) return;

        edit->LvlData.events[i].trigger_timer = qRound(arg1*10);
        edit->LvlData.modified=true;
    }

}

