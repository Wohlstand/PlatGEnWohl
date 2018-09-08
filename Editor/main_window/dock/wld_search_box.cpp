/*
 * Platformer Game Engine by Wohlstand, a free platform for game making
 * Copyright (c) 2014-2017 Vitaly Novichkov <admin@wohlnet.ru>
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

#include <editing/_dialogs/itemselectdialog.h>
#include <editing/_scenes/world/items/item_level.h>
#include <editing/_scenes/world/items/item_music.h>
#include <editing/_scenes/world/items/item_path.h>
#include <editing/_scenes/world/items/item_scene.h>
#include <editing/_scenes/world/items/item_tile.h>
#include <editing/_dialogs/levelfilelist.h>
#include <PGE_File_Formats/wld_filedata.h>

#include <ui_mainwindow.h>
#include <mainwindow.h>

#include "wld_search_box.h"
#include "ui_wld_search_box.h"

WldSearchBox::WldSearchBox(QWidget *parent) :
    QDockWidget(parent),
    MWDock_Base(parent),
    ui(new Ui::WldSearchBox)
{
    setVisible(false);
    setAttribute(Qt::WA_ShowWithoutActivating);
    ui->setupUi(this);
    lockResetWorld = false;

    currentSearches = 0;

    curTerrain.data.id = 0;
    curTerrain.data.meta.index = 0;

    curScenery.data.id = 0;
    curScenery.data.meta.index = 0;

    curPath.data.id = 0;
    curPath.data.meta.index = 0;

    curLevel.data.id = 0;
    curLevel.data.meta.index = 0;

    curMusicBox.data.id = 0;
    curMusicBox.data.meta.index = 0;

    QRect mwg = mw()->geometry();
    int GOffset = 10;
    mw()->addDockWidget(Qt::RightDockWidgetArea, this);
    connect(mw(), SIGNAL(languageSwitched()), this, SLOT(re_translate()));
    setFloating(true);
    setGeometry(
        mwg.right() - width() - GOffset,
        mwg.y() + 120,
        width(),
        height()
    );

    m_lastVisibilityState = isVisible();
    mw()->docks_world.
    addState(this, &m_lastVisibilityState);

    //enable & disable
    connect(ui->Find_Check_TypeLevel, SIGNAL(toggled(bool)), ui->Find_Button_TypeLevel, SLOT(setEnabled(bool)));
    connect(ui->Find_Check_TypeTile, SIGNAL(toggled(bool)), ui->Find_Button_TypeTile, SLOT(setEnabled(bool)));
    connect(ui->Find_Check_TypeScenery, SIGNAL(toggled(bool)), ui->Find_Button_TypeScenery, SLOT(setEnabled(bool)));
    connect(ui->Find_Check_TypePath, SIGNAL(toggled(bool)), ui->Find_Button_TypePath, SLOT(setEnabled(bool)));
    connect(ui->Find_Check_TypeMusic, SIGNAL(toggled(bool)), ui->Find_Button_TypeMusic, SLOT(setEnabled(bool)));
    connect(ui->Find_Check_PathBackground, SIGNAL(toggled(bool)), ui->Find_Check_PathBackgroundActive, SLOT(setEnabled(bool)));
    connect(ui->Find_Check_BigPathBackground, SIGNAL(toggled(bool)), ui->Find_Check_BigPathBackgroundActive, SLOT(setEnabled(bool)));
    connect(ui->Find_Check_AlwaysVisible, SIGNAL(toggled(bool)), ui->Find_Check_AlwaysVisibleActive, SLOT(setEnabled(bool)));
    connect(ui->Find_Check_GameStartPoint, SIGNAL(toggled(bool)), ui->Find_Check_GameStartPointActive, SLOT(setEnabled(bool)));
    connect(ui->Find_Check_LevelFile, SIGNAL(toggled(bool)), ui->Find_Edit_LevelFile, SLOT(setEnabled(bool)));
    connect(ui->Find_Check_LevelFile, SIGNAL(toggled(bool)), ui->Find_Button_LevelFile, SLOT(setEnabled(bool)));
    connect(ui->Find_Check_ContainsTitle, SIGNAL(toggled(bool)), ui->Find_Edit_ContainsTitle, SLOT(setEnabled(bool)));

    //reset if modify
    connect(ui->Find_Button_TypeTile, SIGNAL(clicked()), this, SLOT(resetTileSearch()));
    connect(ui->Find_Button_TypeScenery, SIGNAL(clicked()), this, SLOT(resetScenerySearch()));
    connect(ui->Find_Button_TypePath, SIGNAL(clicked()), this, SLOT(resetPathSearch()));
    connect(ui->Find_Button_TypeLevel, SIGNAL(clicked()), this, SLOT(resetLevelSearch()));
    connect(ui->Find_Button_TypeMusic, SIGNAL(clicked()), this, SLOT(resetMusicSearch()));
    connect(ui->Find_Check_PathBackgroundActive, SIGNAL(clicked()), this, SLOT(resetLevelSearch()));
    connect(ui->Find_Check_BigPathBackgroundActive, SIGNAL(clicked()), this, SLOT(resetLevelSearch()));
    connect(ui->Find_Check_AlwaysVisibleActive, SIGNAL(clicked()), this, SLOT(resetLevelSearch()));
    connect(ui->Find_Check_GameStartPointActive, SIGNAL(clicked()), this, SLOT(resetLevelSearch()));
    connect(ui->Find_Edit_LevelFile, SIGNAL(textEdited(QString)), this, SLOT(resetLevelSearch()));
    connect(ui->Find_Button_LevelFile, SIGNAL(clicked()), this, SLOT(resetLevelSearch()));
    connect(ui->Find_Edit_ContainsTitle, SIGNAL(textEdited(QString)), this, SLOT(resetLevelSearch()));

    //also checkboxes
    connect(ui->Find_Check_TypeTile, SIGNAL(clicked()), this, SLOT(resetTileSearch()));
    connect(ui->Find_Check_TypeScenery, SIGNAL(clicked()), this, SLOT(resetScenerySearch()));
    connect(ui->Find_Check_TypePath, SIGNAL(clicked()), this, SLOT(resetPathSearch()));
    connect(ui->Find_Check_TypeLevel, SIGNAL(clicked()), this, SLOT(resetLevelSearch()));
    connect(ui->Find_Check_TypeMusic, SIGNAL(clicked()), this, SLOT(resetMusicSearch()));
    connect(ui->Find_Check_PathBackground, SIGNAL(clicked()), this, SLOT(resetLevelSearch()));
    connect(ui->Find_Check_BigPathBackground, SIGNAL(clicked()), this, SLOT(resetLevelSearch()));
    connect(ui->Find_Check_AlwaysVisible, SIGNAL(clicked()), this, SLOT(resetLevelSearch()));
    connect(ui->Find_Check_GameStartPoint, SIGNAL(clicked()), this, SLOT(resetLevelSearch()));
    connect(ui->Find_Check_LevelFile, SIGNAL(clicked()), this, SLOT(resetLevelSearch()));
    connect(ui->Find_Check_ContainsTitle, SIGNAL(clicked()), this, SLOT(resetLevelSearch()));

    connect(ui->Find_Button_LevelFile, SIGNAL(clicked()), this, SLOT(selectLevelForSearch()));

    connect(mw()->ui->centralWidget, SIGNAL(subWindowActivated(QMdiSubWindow *)), this, SLOT(toggleNewWindowWLD(QMdiSubWindow *)));
}

WldSearchBox::~WldSearchBox()
{
    delete ui;
}

void WldSearchBox::re_translate()
{
    ui->retranslateUi(this);
}




void MainWindow::on_actionWLD_SearchBox_triggered(bool checked)
{
    dock_WldSearchBox->setVisible(checked);
    if(checked)dock_WldSearchBox->raise();
}

void WldSearchBox::on_WldSearchBox_visibilityChanged(bool visible)
{
    mw()->ui->actionWLD_SearchBox->setChecked(visible);
}

void WldSearchBox::on_Find_Button_ResetLevel_clicked()
{
    if(!(currentSearches & SEARCH_LEVEL))
    {
        ui->Find_Check_TypeLevel->setChecked(true);
        ui->Find_Button_TypeLevel->setText(tr("[empty]"));
        curLevel.data.id = 0;
        ui->Find_Check_PathBackground->setChecked(false);
        ui->Find_Check_PathBackgroundActive->setChecked(false);
        ui->Find_Check_BigPathBackground->setChecked(false);
        ui->Find_Check_BigPathBackgroundActive->setChecked(false);
        ui->Find_Check_AlwaysVisible->setChecked(false);
        ui->Find_Check_AlwaysVisibleActive->setChecked(false);
        ui->Find_Check_GameStartPoint->setChecked(false);
        ui->Find_Check_GameStartPointActive->setChecked(false);
        ui->Find_Check_LevelFile->setChecked(false);
        ui->Find_Edit_LevelFile->setText("");
        ui->Find_Check_ContainsTitle->setChecked(false);
        ui->Find_Edit_ContainsTitle->setText("");
    }
    else
    {
        currentSearches ^= SEARCH_LEVEL;
        ui->Find_Button_ResetLevel->setText(tr("Reset Search Fields"));
        ui->FindStartLevel->setText(tr("Search Level"));
        curLevel.index = 0;
        curLevel.total = 0;
    }
}

void WldSearchBox::on_Find_Button_ResetMusic_clicked()
{
    if(!(currentSearches & SEARCH_MUSICBOX))
    {
        ui->Find_Check_TypeMusic->setChecked(true);
        ui->Find_Button_TypeMusic->setText(tr("[empty]"));
        curMusicBox.data.id = 0;
    }
    else
    {
        currentSearches ^= SEARCH_MUSICBOX;
        ui->Find_Button_ResetMusic->setText(tr("Reset Search Fields"));
        ui->FindStartMusic->setText(tr("Search Music"));
        curMusicBox.index = 0;
        curMusicBox.total = 0;
    }
}

void WldSearchBox::on_Find_Button_ResetPath_clicked()
{
    if(!(currentSearches & SEARCH_PATH))
    {
        ui->Find_Check_TypePath->setChecked(true);
        ui->Find_Button_TypePath->setText(tr("[empty]"));
        curPath.data.id = 0;
    }
    else
    {
        currentSearches ^= SEARCH_PATH;
        ui->Find_Button_ResetPath->setText(tr("Reset Search Fields"));
        ui->FindStartPath->setText(tr("Search Path"));
        curPath.index = 0;
        curPath.total = 0;
    }
}

void WldSearchBox::on_Find_Button_ResetScenery_clicked()
{
    if(!(currentSearches & SEARCH_SCENERY))
    {
        ui->Find_Check_TypeScenery->setChecked(true);
        ui->Find_Button_TypeScenery->setText(tr("[empty]"));
        curScenery.data.id = 0;
    }
    else
    {
        currentSearches ^= SEARCH_SCENERY;
        ui->Find_Button_ResetScenery->setText(tr("Reset Search Fields"));
        ui->FindStartScenery->setText(tr("Search Scenery"));
        curScenery.index = 0;
        curScenery.total = 0;
    }
}

void WldSearchBox::on_Find_Button_ResetTile_clicked()
{
    if(!(currentSearches & SEARCH_TILE))
    {
        ui->Find_Check_TypeTile->setChecked(true);
        ui->Find_Button_TypeTile->setText(tr("[empty]"));
        curTerrain.data.id = 0;
    }
    else
    {
        currentSearches ^= SEARCH_TILE;
        ui->Find_Button_ResetTile->setText(tr("Reset Search Fields"));
        ui->FindStartTile->setText(tr("Search Tile"));
        curTerrain.index = 0;
        curTerrain.total = 0;
    }
}

void WldSearchBox::on_Find_Button_TypeLevel_clicked()
{
    std::unique_ptr<ItemSelectDialog> selLevel(new ItemSelectDialog(&(mw()->configs), ItemSelectDialog::TAB_LEVEL, 0, 0, 0, 0, 0, 0, 0, curLevel.data.id, 0, this));
    if(selLevel->exec() == QDialog::Accepted)
    {
        int selected = selLevel->levelID;
        curLevel.data.id = selected;
        ui->Find_Button_TypeLevel->setText(((selected > 0) ? QString("Level-%1").arg(selected) : tr("[empty]")));
    }
}

void WldSearchBox::on_Find_Button_TypeTile_clicked()
{
    ItemSelectDialog selTile(&(mw()->configs), ItemSelectDialog::TAB_TILE, 0, 0, 0, 0, curTerrain.data.id, 0, 0, 0, 0, this);
    if(selTile.exec() == QDialog::Accepted)
    {
        int selected = selTile.tileID;
        curTerrain.data.id = selected;
        ui->Find_Button_TypeTile->setText(((selected > 0) ? QString("Tile-%1").arg(selected) : tr("[empty]")));
    }
}

void WldSearchBox::on_Find_Button_TypeScenery_clicked()
{
    ItemSelectDialog selScenery(&(mw()->configs), ItemSelectDialog::TAB_SCENERY, 0, 0, 0, 0, 0, curScenery.data.id, 0, 0, 0, this);
    if(selScenery.exec() == QDialog::Accepted)
    {
        int selected = selScenery.sceneryID;
        curScenery.data.id = selected;
        ui->Find_Button_TypeScenery->setText(((selected > 0) ? QString("Scenery-%1").arg(selected) : tr("[empty]")));
    }
}

void WldSearchBox::on_Find_Button_TypePath_clicked()
{
    ItemSelectDialog selPath(&(mw()->configs), ItemSelectDialog::TAB_PATH, 0, 0, 0, 0, 0, 0, curPath.data.id, 0, 0, this);
    if(selPath.exec() == QDialog::Accepted)
    {
        int selected = selPath.pathID;
        curPath.data.id = selected;
        ui->Find_Button_TypePath->setText(((selected > 0) ? QString("Path-%1").arg(selected) : tr("[empty]")));
    }
}

void WldSearchBox::on_Find_Button_TypeMusic_clicked()
{
    ItemSelectDialog selMusic(&(mw()->configs), ItemSelectDialog::TAB_MUSIC, 0, 0, 0, 0, 0, 0, 0, 0, curMusicBox.data.id, this);
    if(selMusic.exec() == QDialog::Accepted)
    {
        int selected = selMusic.musicID;
        curMusicBox.data.id = selected;
        int index = mw()->configs.getMusWldI(selected);
        if(index != -1)
            ui->Find_Button_TypeMusic->setText(mw()->configs.main_music_wld[index].name);
        else
            ui->Find_Button_TypeMusic->setText(((selected > 0) ? QString("Music-%1").arg(selected) : tr("[empty]")));
    }
}


void WldSearchBox::on_FindStartLevel_clicked()
{
    if(!(currentSearches & SEARCH_LEVEL))  //start search
    {
        if(mw()->activeChildWindow() == MainWindow::WND_World)
        {
            currentSearches |= SEARCH_LEVEL;
            ui->FindStartLevel->setText(tr("Next Level"));
            ui->Find_Button_ResetLevel->setText(tr("Stop Search"));
            WorldEdit *edit = mw()->activeWldEditWin();
            if(!edit) return;
            if(doSearchLevel(edit))
            {
                currentSearches ^= SEARCH_LEVEL;
                ui->Find_Button_ResetLevel->setText(tr("Reset Search Fields"));
                ui->FindStartLevel->setText(tr("Search Level"));
                QMessageBox::information(this, tr("Search Complete"), tr("Level search completed!"));
            }
        }
    }
    else   //middle in a search
    {
        if(mw()->activeChildWindow() == MainWindow::WND_World)
        {
            WorldEdit *edit = mw()->activeWldEditWin();
            if(!edit) return;
            if(doSearchLevel(edit))
            {
                currentSearches ^= SEARCH_LEVEL;
                ui->Find_Button_ResetLevel->setText(tr("Reset Search Fields"));
                ui->FindStartLevel->setText(tr("Search Level"));
                QMessageBox::information(this, tr("Search Complete"), tr("Level search completed!"));
            }
        }
    }
}

void WldSearchBox::on_FindStartTile_clicked()
{
    if(!(currentSearches & SEARCH_TILE))  //start search
    {
        if(mw()->activeChildWindow() == MainWindow::WND_World)
        {
            currentSearches |= SEARCH_TILE;
            ui->FindStartTile->setText(tr("Next Tile"));
            ui->Find_Button_ResetTile->setText(tr("Stop Search"));
            WorldEdit *edit = mw()->activeWldEditWin();
            if(!edit) return;
            if(doSearchTile(edit))
            {
                currentSearches ^= SEARCH_TILE;
                ui->Find_Button_ResetTile->setText(tr("Reset Search Fields"));
                ui->FindStartTile->setText(tr("Search Tile"));
                QMessageBox::information(this, tr("Search Complete"), tr("Tile search completed!"));
            }
        }
    }
    else   //middle in a search
    {
        if(mw()->activeChildWindow() == MainWindow::WND_World)
        {
            WorldEdit *edit = mw()->activeWldEditWin();
            if(!edit) return;
            if(doSearchTile(edit))
            {
                currentSearches ^= SEARCH_TILE;
                ui->Find_Button_ResetTile->setText(tr("Reset Search Fields"));
                ui->FindStartTile->setText(tr("Search Tile"));
                QMessageBox::information(this, tr("Search Complete"), tr("Tile search completed!"));
            }
        }
    }
}

void WldSearchBox::on_FindStartScenery_clicked()
{
    if(!(currentSearches & SEARCH_SCENERY))  //start search
    {
        if(mw()->activeChildWindow() == MainWindow::WND_World)
        {
            currentSearches |= SEARCH_SCENERY;
            ui->FindStartScenery->setText(tr("Next Scenery"));
            ui->Find_Button_ResetScenery->setText(tr("Stop Search"));
            WorldEdit *edit = mw()->activeWldEditWin();
            if(!edit) return;
            if(doSearchScenery(edit))
            {
                currentSearches ^= SEARCH_SCENERY;
                ui->Find_Button_ResetScenery->setText(tr("Reset Search Fields"));
                ui->FindStartScenery->setText(tr("Search Scenery"));
                QMessageBox::information(this, tr("Search Complete"), tr("Scenery search completed!"));
            }
        }
    }
    else   //middle in a search
    {
        if(mw()->activeChildWindow() == MainWindow::WND_World)
        {
            WorldEdit *edit = mw()->activeWldEditWin();
            if(!edit) return;
            if(doSearchScenery(edit))
            {
                currentSearches ^= SEARCH_SCENERY;
                ui->Find_Button_ResetScenery->setText(tr("Reset Search Fields"));
                ui->FindStartScenery->setText(tr("Search Scenery"));
                QMessageBox::information(this, tr("Search Complete"), tr("Scenery search completed!"));
            }
        }
    }
}

void WldSearchBox::on_FindStartPath_clicked()
{
    if(!(currentSearches & SEARCH_PATH))  //start search
    {
        if(mw()->activeChildWindow() == MainWindow::WND_World)
        {
            currentSearches |= SEARCH_PATH;
            ui->FindStartPath->setText(tr("Next Path"));
            ui->Find_Button_ResetPath->setText(tr("Stop Search"));
            WorldEdit *edit = mw()->activeWldEditWin();
            if(!edit) return;
            if(doSearchPath(edit))
            {
                currentSearches ^= SEARCH_PATH;
                ui->Find_Button_ResetPath->setText(tr("Reset Search Fields"));
                ui->FindStartPath->setText(tr("Search Path"));
                QMessageBox::information(this, tr("Search Complete"), tr("Path search completed!"));
            }
        }
    }
    else   //middle in a search
    {
        if(mw()->activeChildWindow() == MainWindow::WND_World)
        {
            WorldEdit *edit = mw()->activeWldEditWin();
            if(!edit) return;
            if(doSearchPath(edit))
            {
                currentSearches ^= SEARCH_PATH;
                ui->Find_Button_ResetPath->setText(tr("Reset Search Fields"));
                ui->FindStartPath->setText(tr("Search Path"));
                QMessageBox::information(this, tr("Search Complete"), tr("Path search completed!"));
            }
        }
    }
}

void WldSearchBox::on_FindStartMusic_clicked()
{
    if(!(currentSearches & SEARCH_MUSICBOX))  //start search
    {
        if(mw()->activeChildWindow() == MainWindow::WND_World)
        {
            currentSearches |= SEARCH_MUSICBOX;
            ui->FindStartMusic->setText(tr("Next Music"));
            ui->Find_Button_ResetMusic->setText(tr("Stop Search"));
            WorldEdit *edit = mw()->activeWldEditWin();
            if(!edit) return;
            if(doSearchMusic(edit))
            {
                currentSearches ^= SEARCH_MUSICBOX;
                ui->Find_Button_ResetMusic->setText(tr("Reset Search Fields"));
                ui->FindStartMusic->setText(tr("Search Music"));
                QMessageBox::information(this, tr("Search Complete"), tr("Music search completed!"));
            }
        }
    }
    else   //middle in a search
    {
        if(mw()->activeChildWindow() == MainWindow::WND_World)
        {
            WorldEdit *edit = mw()->activeWldEditWin();
            if(!edit) return;
            if(doSearchMusic(edit))
            {
                currentSearches ^= SEARCH_MUSICBOX;
                ui->Find_Button_ResetMusic->setText(tr("Reset Search Fields"));
                ui->FindStartMusic->setText(tr("Search Music"));
                QMessageBox::information(this, tr("Search Complete"), tr("Music search completed!"));
            }
        }
    }
}

bool WldSearchBox::doSearchTile(WorldEdit *edit)
{
    QList<QGraphicsItem *> gr = edit->scene->items();
    quint64 grSize = static_cast<quint64>(gr.size());

    //Reset search on changing total number of elements (something added or deleted)
    //... however, will be continued if something replaced
    if(grSize != curTerrain.total)
    {
        curTerrain.total = grSize;
        curTerrain.index = 0;
    }

    if(curTerrain.data.meta.index < grSize)
    {
        for(quint64 i = curTerrain.index; i < grSize; ++i)
        {
            if(gr[i]->data(ITEM_TYPE).toString() == "TILE")
            {
                bool toBeFound = true;
                if(ui->Find_Check_TypeTile->isChecked() && curTerrain.data.id != 0 && toBeFound)
                    toBeFound = ((ItemTile *)gr[i])->m_data.id == (unsigned int)curTerrain.data.id;
                if(toBeFound)
                {
                    foreach(QGraphicsItem *i, edit->scene->selectedItems())
                        i->setSelected(false);
                    gr[i]->setSelected(true);
                    edit->goTo(((ItemTile *)gr[i])->m_data.x, ((ItemTile *)gr[i])->m_data.y, false, QPoint(-300, -300));
                    curTerrain.index = i + 1;
                    return false;
                }
            }
        }
    }

    //end search
    curTerrain.index = 0;
    return true;
}

bool WldSearchBox::doSearchScenery(WorldEdit *edit)
{
    QList<QGraphicsItem *> gr = edit->scene->items();
    quint64 grSize = static_cast<quint64>(gr.size());

    //Reset search on changing total number of elements (something added or deleted)
    //... however, will be continued if something replaced
    if(grSize != curScenery.total)
    {
        curScenery.total = grSize;
        curScenery.index = 0;
    }

    if(curScenery.index  < grSize)
    {
        for(quint64 i = curScenery.index; i < grSize; ++i)
        {
            if(gr[i]->data(ITEM_TYPE).toString() == "SCENERY")
            {
                bool toBeFound = true;
                if(ui->Find_Check_TypeScenery->isChecked() && curScenery.data.id != 0 && toBeFound)
                    toBeFound = ((ItemScene *)gr[i])->m_data.id == (unsigned int)curScenery.data.id;
                if(toBeFound)
                {
                    foreach(QGraphicsItem *i, edit->scene->selectedItems())
                        i->setSelected(false);
                    gr[i]->setSelected(true);
                    edit->goTo(((ItemScene *)gr[i])->m_data.x, ((ItemScene *)gr[i])->m_data.y, false, QPoint(-300, -300));
                    curScenery.index = i + 1;
                    return false;
                }
            }
        }
    }

    //end search
    curScenery.index = 0;
    return true;
}

bool WldSearchBox::doSearchPath(WorldEdit *edit)
{
    QList<QGraphicsItem *> gr = edit->scene->items();
    quint64 grSize = static_cast<quint64>(gr.size());

    //Reset search on changing total number of elements (something added or deleted)
    //... however, will be continued if something replaced
    if(grSize != curPath.total)
    {
        curPath.total = grSize;
        curPath.index = 0;
    }

    if(curPath.index < grSize)
    {
        for(quint64 i = curPath.index; i < grSize; ++i)
        {
            if(gr[i]->data(ITEM_TYPE).toString() == "PATH")
            {
                bool toBeFound = true;
                if(ui->Find_Check_TypePath->isChecked() && curPath.data.id != 0 && toBeFound)
                    toBeFound = ((ItemPath *)gr[i])->m_data.id == (unsigned int)curPath.data.id;
                if(toBeFound)
                {
                    foreach(QGraphicsItem *i, edit->scene->selectedItems())
                        i->setSelected(false);
                    gr[i]->setSelected(true);
                    edit->goTo(((ItemPath *)gr[i])->m_data.x, ((ItemPath *)gr[i])->m_data.y, false, QPoint(-300, -300));
                    curPath.index = i + 1;
                    return false;
                }
            }
        }
    }

    //end search
    curPath.index = 0;
    return true;
}

bool WldSearchBox::doSearchLevel(WorldEdit *edit)
{
    QList<QGraphicsItem *> gr = edit->scene->items();
    quint64 grSize = static_cast<quint64>(gr.size());

    //Reset search on changing total number of elements (something added or deleted)
    //... however, will be continued if something replaced
    if(grSize != curLevel.total)
    {
        curLevel.total = grSize;
        curLevel.index = 0;
    }

    if(curLevel.index < grSize)
    {
        for(quint64 i = curLevel.index; i < grSize; ++i)
        {
            if(gr[i]->data(ITEM_TYPE).toString() == "LEVEL")
            {
                bool toBeFound = true;
                if(ui->Find_Check_TypeLevel->isChecked() && curLevel.data.id != 0 && toBeFound)
                    toBeFound = ((ItemLevel *)gr[i])->m_data.id == (unsigned int)curLevel.data.id;
                if(ui->Find_Check_PathBackground->isChecked() && toBeFound)
                    toBeFound = ((ItemLevel *)gr[i])->m_data.pathbg == ui->Find_Check_PathBackgroundActive->isChecked();
                if(ui->Find_Check_BigPathBackground->isChecked() && toBeFound)
                    toBeFound = ((ItemLevel *)gr[i])->m_data.bigpathbg == ui->Find_Check_BigPathBackgroundActive->isChecked();
                if(ui->Find_Check_AlwaysVisible->isChecked() && toBeFound)
                    toBeFound = ((ItemLevel *)gr[i])->m_data.alwaysVisible == ui->Find_Check_AlwaysVisibleActive->isChecked();
                if(ui->Find_Check_GameStartPoint->isChecked() && toBeFound)
                    toBeFound = ((ItemLevel *)gr[i])->m_data.gamestart == ui->Find_Check_GameStartPointActive->isChecked();
                if(ui->Find_Check_LevelFile->isChecked() && toBeFound)
                    toBeFound = ((ItemLevel *)gr[i])->m_data.lvlfile.contains(ui->Find_Edit_LevelFile->text(), Qt::CaseInsensitive);
                if(ui->Find_Check_ContainsTitle->isChecked() && toBeFound)
                    toBeFound = ((ItemLevel *)gr[i])->m_data.title.contains(ui->Find_Edit_ContainsTitle->text(), Qt::CaseInsensitive);
                if(toBeFound)
                {
                    foreach(QGraphicsItem *i, edit->scene->selectedItems())
                        i->setSelected(false);
                    gr[i]->setSelected(true);
                    edit->goTo(((ItemLevel *)gr[i])->m_data.x, ((ItemLevel *)gr[i])->m_data.y, false, QPoint(-300, -300));
                    curLevel.index = i + 1;
                    return false;
                }
            }
        }
    }

    //end search
    curLevel.index = 0;
    return true;
}

bool WldSearchBox::doSearchMusic(WorldEdit *edit)
{
    QList<QGraphicsItem *> gr = edit->scene->items();
    quint64 grSize = static_cast<quint64>(gr.size());

    //Reset search on changing total number of elements (something added or deleted)
    //... however, will be continued if something replaced
    if(grSize != curMusicBox.total)
    {
        curMusicBox.total = grSize;
        curMusicBox.index = 0;
    }

    if(curMusicBox.index < grSize)
    {
        for(quint64 i = curMusicBox.index; i < grSize; ++i)
        {
            if(gr[i]->data(ITEM_TYPE).toString() == "MUSICBOX")
            {
                bool toBeFound = true;
                if(ui->Find_Check_TypeMusic->isChecked() && curMusicBox.data.id != 0 && toBeFound)
                    toBeFound = ((ItemMusic *)gr[i])->m_data.id == (unsigned int)curMusicBox.data.id;
                if(toBeFound)
                {
                    foreach(QGraphicsItem *i, edit->scene->selectedItems())
                        i->setSelected(false);
                    gr[i]->setSelected(true);
                    edit->goTo(((ItemMusic *)gr[i])->m_data.x, ((ItemMusic *)gr[i])->m_data.y, false, QPoint(-300, -300));
                    curMusicBox.index = i + 1;
                    return false;
                }
            }
        }
    }

    //end search
    curMusicBox.index = 0;
    return true;
}

void WldSearchBox::resetAllSearchFieldsWLD()
{
    resetAllSearchesWLD();

    on_Find_Button_ResetTile_clicked();
    on_Find_Button_ResetScenery_clicked();
    on_Find_Button_ResetPath_clicked();
    on_Find_Button_ResetLevel_clicked();
    on_Find_Button_ResetMusic_clicked();
}

void WldSearchBox::resetAllSearchesWLD()
{
    resetTileSearch();
    resetScenerySearch();
    resetPathSearch();
    resetLevelSearch();
    resetMusicSearch();
}

void WldSearchBox::resetTileSearch()
{
    if(lockResetWorld) return;
    lockResetWorld = true;
    if(currentSearches & SEARCH_TILE)
        on_Find_Button_ResetTile_clicked();
    lockResetWorld = false;
}

void WldSearchBox::resetScenerySearch()
{
    if(lockResetWorld) return;
    lockResetWorld = true;
    if(currentSearches & SEARCH_SCENERY)
        on_Find_Button_ResetScenery_clicked();
    lockResetWorld = false;
}

void WldSearchBox::resetPathSearch()
{
    if(lockResetWorld) return;
    lockResetWorld = true;
    if(currentSearches & SEARCH_PATH)
        on_Find_Button_ResetPath_clicked();
    lockResetWorld = false;
}

void WldSearchBox::resetLevelSearch()
{
    if(lockResetWorld) return;
    lockResetWorld = true;
    if(currentSearches & SEARCH_LEVEL)
        on_Find_Button_ResetLevel_clicked();
    lockResetWorld = false;
}

void WldSearchBox::resetMusicSearch()
{
    if(lockResetWorld) return;
    lockResetWorld = true;
    if(currentSearches & SEARCH_MUSICBOX)
        on_Find_Button_ResetMusic_clicked();
    lockResetWorld = false;
}

void WldSearchBox::toggleNewWindowWLD(QMdiSubWindow */*window*/)
{
    resetAllSearchesWLD();
}

void WldSearchBox::selectLevelForSearch()
{
    QString dirPath;
    if(mw()->activeChildWindow() == MainWindow::WND_World)
    {
        WorldEdit *edit = mw()->activeWldEditWin();
        if(!edit) return;
        dirPath = edit->WldData.meta.path;
    }
    else
        return;

    LevelFileList levelList(dirPath, ui->Find_Edit_LevelFile->text());
    if(levelList.exec() == QDialog::Accepted)
        ui->Find_Edit_LevelFile->setText(levelList.SelectedFile);
}
