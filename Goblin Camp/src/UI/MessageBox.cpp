/* Copyright 2010 Ilkka Halila
 This file is part of Goblin Camp.
 
 Goblin Camp is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 Goblin Camp is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License 
 along with Goblin Camp. If not, see <http://www.gnu.org/licenses/>.*/
#include "stdafx.hpp"

#include <string>

#include "UI/MessageBox.hpp"
#include "UI/Dialog.hpp"
#include "UI/Label.hpp"
#include "UI/Button.hpp"

void MessageBox::ShowMessageBox(std::string text, boost::function<void()> action, std::string button) {
	UIContainer *contents = new UIContainer(std::vector<Drawable *>(), 0, 0, 54, (text.length() / 50) + 8);
	Dialog *dialog = new Dialog(contents, "", 54, (text.length() / 50) + 8);
	int i = 0;
	do {
		contents->AddComponent(new Label(text.substr(i, 50), 27, 2+(i/50)));
		i += 50;
	} while (i < text.length());
	contents->AddComponent(new Button(button, action, 22, (i/50)+3, 10, 'o', true));
	dialog->ShowModal();
}

