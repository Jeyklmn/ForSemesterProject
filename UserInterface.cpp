#include "UserInterface.h"
#include <iostream> // ��� �������
#include <algorithm> // ��� std::min_element, std::max_element

#if defined(_MSC_VER)
#pragma execution_character_set("utf-8")
#endif

// --- ��������������� ������� ��� �������� ������ ����� ---
std::pair<tgui::Label::Ptr, tgui::EditBox::Ptr> createInputRowControls(const sf::String& labelText, float editBoxWidth, float rowHeight) {
    auto label = tgui::Label::create(tgui::String(labelText)); // sf::String � L"" ������ �������� � tgui::Label
    if (label) {
        label->getRenderer()->setTextColor(tgui::Color::Black);
        label->setVerticalAlignment(tgui::Label::VerticalAlignment::Center);
    }

    auto editBox = tgui::EditBox::create();
    if (editBox) {
        editBox->setSize({ editBoxWidth, rowHeight });
    }
    return { label, editBox };
}

// --- ����������� � ������������� ---
UserInterface::UserInterface()
    : m_window({ 1200, 800 }, L"������ ���������� �������� ����"),
    m_gui(m_window),
    m_trajectoryAvailable(false) {

    m_gui.setFont("arial.ttf");

    // �������� ������ ��� SFML (������������ �� Canvas)
    if (!m_sfmlFont.loadFromFile("arial.ttf")) {
        std::cerr << "SFML: Error - Failed to load font 'arial.ttf' for SFML rendering!\n";
    }

    initializeGui();
}

void UserInterface::initializeGui() {
    std::cout << "DEBUG: Initializing GUI..." << std::endl;
    loadWidgets();
    setupLayout(); // �������� setupLayout ����� loadWidgets
    setupLayout();
    connectSignals();
    populateTable({}); // ��������� ������ ��������� �������
    std::cout << "DEBUG: GUI Initialized." << std::endl;
}

//void UserInterface::resetTguiCanvasView() {
//    if (!m_trajectoryCanvas) {
//        std::cerr << "Error: m_trajectoryCanvas is null in resetTguiCanvasView." << std::endl;
//        return;
//    }
//    sf::RenderTexture& rt = m_trajectoryCanvas->getRenderTexture();
//    if (rt.getSize().x == 0 || rt.getSize().y == 0) {
//        std::cout << "Warning: TGUI Canvas RenderTarget has zero size in resetTguiCanvasView. Using default view." << std::endl;
//        m_tguiCanvasView = rt.getDefaultView(); // ���������� ����������� ��� �� ���������
//        m_tguiCanvasCurrentScaleFactor = 1.0f; // ������������� ���������� ���� � getDefaultView
//    }
//    else {
//        m_tguiCanvasView.setSize(static_cast<sf::Vector2f>(rt.getSize()));
//        m_tguiCanvasView.setCenter(0.f, 0.f); // ������� (0,0) � ������ View
//        m_tguiCanvasView.zoom(1.0f / TGUI_CANVAS_INITIAL_SCALE_FACTOR);
//        m_tguiCanvasCurrentScaleFactor = TGUI_CANVAS_INITIAL_SCALE_FACTOR;
//    }
//    std::cout << "DEBUG: TGUI Canvas View reset/initialized." << std::endl;
//    // prepareTrajectoryForDisplay(); // �������� ����� ��������� ����� m_calculatedStates
//}

// --- �������� �������� ---
void UserInterface::loadWidgets() {
    std::cout << "DEBUG: Loading all widgets..." << std::endl;
    loadLeftPanelWidgets();
    loadRightPanelWidgets();
    std::cout << "DEBUG: All widgets loaded." << std::endl;
}

void UserInterface::loadLeftPanelWidgets() {
    m_leftPanel = tgui::Panel::create();
    if (!m_leftPanel) { std::cerr << "Error: Failed to create m_leftPanel" << std::endl; return; }
    m_leftPanel->getRenderer()->setBackgroundColor(tgui::Color(220, 220, 220));
    m_leftPanel->getRenderer()->setBorders({ 1, 1, 1, 1 });
    m_leftPanel->getRenderer()->setBorderColor(tgui::Color::Black);
    m_gui.add(m_leftPanel);

    // 1. ��������� "�������� ��������"
    m_inputTitleLabel = tgui::Label::create(L"�������� ��������");
    if (!m_inputTitleLabel) { std::cerr << "Error: Failed to create m_inputTitleLabel" << std::endl; return; }
    m_inputTitleLabel->getRenderer()->setTextStyle(tgui::TextStyle::Bold);
    m_inputTitleLabel->setHorizontalAlignment(tgui::Label::HorizontalAlignment::Center);
    m_inputTitleLabel->getRenderer()->setTextColor(tgui::Color::Black);
    m_inputTitleLabel->setSize({ "100% - " + tgui::String::fromNumber(2 * PANEL_PADDING), TITLE_HEIGHT });
    m_inputTitleLabel->setPosition({ PANEL_PADDING, PANEL_PADDING });
    m_leftPanel->add(m_inputTitleLabel);

    // 2. Grid ��� ����� �����
    m_inputControlsGrid = tgui::Grid::create(); // ���������� ���� ������
    if (!m_inputControlsGrid) { std::cerr << "Error: Failed to create m_inputControlsGrid" << std::endl; return; }
    m_inputControlsGrid->setPosition({ PANEL_PADDING, tgui::bindBottom(m_inputTitleLabel) + WIDGET_SPACING });
    m_leftPanel->add(m_inputControlsGrid);

    unsigned int currentRow = 0;
    // ������ ��� ���������� ������ � inputControlsGrid
    auto addInputRowToGrid = [&](const sf::String& text, tgui::EditBox::Ptr& editBoxMember) {
        auto pair = createInputRowControls(text, INPUT_FIELD_WIDTH, INPUT_ROW_HEIGHT);
        if (!pair.first || !pair.second) {
            std::cerr << "Error: Failed to create input pair for: " << text.toAnsiString() << std::endl;
            return;
        }
        editBoxMember = pair.second;
        m_inputControlsGrid->addWidget(pair.first, currentRow, 0);
        m_inputControlsGrid->addWidget(editBoxMember, currentRow, 1);
        m_inputControlsGrid->setWidgetPadding(currentRow, 0, { 5, 5, 5, 0 });  // Label: T,R,B,L (0 �����, �.�. ���� ����� ���� ������)
        m_inputControlsGrid->setWidgetPadding(currentRow, 1, { 5, 0, 5, 5 });  // EditBox: T,R,B,L (0 ������)
        currentRow++;
    };

    addInputRowToGrid(L"m (�����, ��):", m_edit_m);
    addInputRowToGrid(L"M (�����, ��):", m_edit_M);
    addInputRowToGrid(L"V0 (��������, �/�):", m_edit_V0);
    addInputRowToGrid(L"T (�����, ���):", m_edit_T);
    addInputRowToGrid(L"k (������������):", m_edit_k);
    addInputRowToGrid(L"F (������������):", m_edit_F);

    // 3. ������ "���������� ����������!"
    m_calculateButton = tgui::Button::create(L"���������� ����������!");
    if (!m_calculateButton) { std::cerr << "Error: Failed to create m_calculateButton" << std::endl; return; }
    m_calculateButton->getRenderer()->setRoundedBorderRadius(15);
    m_calculateButton->setSize({ "100% - " + tgui::String::fromNumber(2 * PANEL_PADDING), 40 });
    m_calculateButton->setPosition({ PANEL_PADDING, tgui::bindBottom(m_inputControlsGrid) + WIDGET_SPACING * 2 }); // ������� ������ ��� ������
    m_leftPanel->add(m_calculateButton);
}

void UserInterface::loadRightPanelWidgets() {
    m_rightPanel = tgui::Panel::create();
    if (!m_rightPanel) { std::cerr << "Error: Failed to create m_rightPanel" << std::endl; return; }
    m_gui.add(m_rightPanel); // ������� ���������, ����� ����������� ����������

    loadTrajectoryWidgets(m_rightPanel);
    loadTableWidgets(m_rightPanel);
}

void UserInterface::loadTrajectoryWidgets(tgui::Panel::Ptr parentPanel) {
    m_trajectoryContainerPanel = tgui::Panel::create();
    if (!m_trajectoryContainerPanel) { std::cerr << "Error: Failed to create m_trajectoryContainerPanel" << std::endl; return; }
    m_trajectoryContainerPanel->getRenderer()->setBorders({ 1,1,1,1 });
    m_trajectoryContainerPanel->getRenderer()->setBorderColor(tgui::Color::Black);
    m_trajectoryContainerPanel->getRenderer()->setBackgroundColor(tgui::Color::White);
    parentPanel->add(m_trajectoryContainerPanel);

    m_trajectoryTitleLabel = tgui::Label::create(L"���������� �������� ����");
    if (!m_trajectoryTitleLabel) { std::cerr << "Error: Failed to create m_trajectoryTitleLabel" << std::endl; return; }
    m_trajectoryTitleLabel->getRenderer()->setTextStyle(tgui::TextStyle::Bold);
    m_trajectoryTitleLabel->setHorizontalAlignment(tgui::Label::HorizontalAlignment::Center);
    m_trajectoryTitleLabel->getRenderer()->setTextColor(tgui::Color::Black);
    m_trajectoryTitleLabel->setSize({ "100%", TITLE_HEIGHT });
    m_trajectoryContainerPanel->add(m_trajectoryTitleLabel, "TrajectoryTitle"); // ���������� ��� ��� ���������������� �������

    m_trajectoryCanvas = tgui::Canvas::create();
    if (!m_trajectoryCanvas) { std::cerr << "Error: Failed to create m_trajectoryCanvas" << std::endl; return; }
    m_trajectoryCanvas->setSize({ "100%", "100% - " + tgui::String::fromNumber(TITLE_HEIGHT) });
    m_trajectoryCanvas->setPosition({ 0, "TrajectoryTitle.bottom" });
    m_trajectoryContainerPanel->add(m_trajectoryCanvas);
}

void UserInterface::loadTableWidgets(tgui::Panel::Ptr parentPanel) {
    m_tableContainerPanel = tgui::Panel::create();
    if (!m_tableContainerPanel) { std::cerr << "Error: Failed to create m_tableContainerPanel" << std::endl; return; }
    m_tableContainerPanel->getRenderer()->setBorders({ 1,1,1,1 });
    m_tableContainerPanel->getRenderer()->setBorderColor(tgui::Color::Black);
    m_tableContainerPanel->getRenderer()->setBackgroundColor(tgui::Color::White);
    parentPanel->add(m_tableContainerPanel);

    m_tableTitleLabel = tgui::Label::create(L"������� ��������� � ���������");
    if (!m_tableTitleLabel) { std::cerr << "Error: Failed to create m_tableTitleLabel" << std::endl; return; }
    m_tableTitleLabel->getRenderer()->setTextStyle(tgui::TextStyle::Bold);
    m_tableTitleLabel->setHorizontalAlignment(tgui::Label::HorizontalAlignment::Center);
    m_tableTitleLabel->getRenderer()->setTextColor(tgui::Color::Black);
    m_tableTitleLabel->setSize({ "100%", TITLE_HEIGHT });
    m_tableContainerPanel->add(m_tableTitleLabel, "TableTitle");

    m_tableHeaderGrid = tgui::Grid::create();
    if (!m_tableHeaderGrid) { std::cerr << "Error: Failed to create m_tableHeaderGrid" << std::endl; return; }
    m_tableHeaderGrid->setSize({ "100% - " + tgui::String::fromNumber(SCROLLBAR_WIDTH_ESTIMATE), HEADER_HEIGHT });
    m_tableHeaderGrid->setPosition({ 0, "TableTitle.bottom" });

    std::vector<sf::String> headers = { L"h, ���", L"x", L"y", L"Vx", L"Vy" };
    for (size_t i = 0; i < headers.size(); ++i) {
        auto headerLabel = tgui::Label::create(tgui::String(headers[i]));
        if (!headerLabel) { std::cerr << "Error: Failed to create headerLabel " << i << std::endl; continue; }
        headerLabel->getRenderer()->setTextColor(tgui::Color::Black);
        headerLabel->getRenderer()->setBorders({ 0,0,0,1 }); // ������ ������ �������
        headerLabel->getRenderer()->setBorderColor(tgui::Color::Black);
        headerLabel->setHorizontalAlignment(tgui::Label::HorizontalAlignment::Center);
        headerLabel->setVerticalAlignment(tgui::Label::VerticalAlignment::Center);
        m_tableHeaderGrid->addWidget(headerLabel, 0, i);
        // ����� �������� ������� ��� headerLabel, ���� �����
        // m_tableHeaderGrid->setWidgetPadding(0, i, {2,5,2,5}); // T,R,B,L
    }
    m_tableContainerPanel->add(m_tableHeaderGrid);

    m_tableDataPanel = tgui::ScrollablePanel::create();
    if (!m_tableDataPanel) { std::cerr << "Error: Failed to create m_tableDataPanel" << std::endl; return; }
    m_tableDataPanel->setSize({ "100%", "100% - " + tgui::String::fromNumber(TITLE_HEIGHT + HEADER_HEIGHT) });
    m_tableDataPanel->setPosition({ 0, tgui::bindBottom(m_tableHeaderGrid) });
    m_tableDataPanel->getRenderer()->setBackgroundColor(tgui::Color(245, 245, 245));
    m_tableContainerPanel->add(m_tableDataPanel);

    m_tableDataGrid = tgui::Grid::create();
    if (!m_tableDataGrid) { std::cerr << "Error: Failed to create m_tableDataGrid for data" << std::endl; return; }
    // ������ � ������ m_tableDataGrid ����� ����������� ScrollablePanel � ��� ����������
    m_tableDataPanel->add(m_tableDataGrid);
}

// --- ���������� ---
void UserInterface::setupLayout() {
    std::cout << "DEBUG: Setting up layout..." << std::endl;
    // ����� ������
    m_leftPanel->setSize({ "30%", "100%" }); // ������� ���� ��� ��������
    m_leftPanel->setPosition({ 0, 0 });

    // ������ ������
    m_rightPanel->setSize({ "70%", "100%" });
    m_rightPanel->setPosition({ "30%", 0 });

    // ���������� ������ ������ ������
    const float rightPanelPadding = PANEL_PADDING;
    const float verticalSpacing = WIDGET_SPACING / 2.f;

    m_trajectoryContainerPanel->setSize(
        { tgui::bindWidth(m_rightPanel) - 2 * rightPanelPadding,
         "60% - " + tgui::String::fromNumber(rightPanelPadding + verticalSpacing / 2.f) 
        }
    );
    m_trajectoryContainerPanel->setPosition({ rightPanelPadding, rightPanelPadding });

    m_tableContainerPanel->setSize(
        { tgui::bindWidth(m_rightPanel) - 2 * rightPanelPadding,
         "40% - " + tgui::String::fromNumber(rightPanelPadding + verticalSpacing / 2.f) 
        }
    );
    m_tableContainerPanel->setPosition({ rightPanelPadding, tgui::bindBottom(m_trajectoryContainerPanel) + verticalSpacing });
    std::cout << "DEBUG: Layout setup finished." << std::endl;
}

// --- ����������� �������� ---
void UserInterface::connectSignals() {
    if (m_calculateButton) {
        // ���������� .connect() ��� TGUI 0.9.x
        m_calculateButton->onPress.connect(&UserInterface::onCalculateButtonPressed, this);
    }
    else {
        std::cerr << "Error: m_calculateButton is null in connectSignals! Cannot connect." << std::endl;
    }
}

// --- ����������� � ������ ---
void UserInterface::onCalculateButtonPressed() {
    std::cout << "Calculate button pressed!" << std::endl;
    
    SimulationParameters paramsFromUI;
    
    try {
        if (m_edit_M && !m_edit_M->getText().empty()) paramsFromUI.M = std::stod(m_edit_M->getText().toStdString());
        if (m_edit_V0 && !m_edit_V0->getText().empty()) {
            double v0_val = std::stod(m_edit_V0->getText().toStdString());
            paramsFromUI.initialState.vy = v0_val;
            paramsFromUI.initialState.vx = 0.0;
        }
        if (m_edit_T && !m_edit_T->getText().empty()) {
            double total_time = std::stod(m_edit_T->getText().toStdString());
            if (paramsFromUI.DT > 0.000001) { // ������ �� ������� �� ����� ����� ����� ��� ����
                paramsFromUI.STEPS = static_cast<int>(total_time / paramsFromUI.DT);
                if (paramsFromUI.STEPS <= 0) paramsFromUI.STEPS = 1;
            }
            else {
                paramsFromUI.STEPS = 1000; // �������� �� ���������, ���� DT �����������
                std::cerr << "Warning: Invalid DT, using default STEPS." << std::endl;
            }
        }
        if (m_edit_k && !m_edit_k->getText().empty()) paramsFromUI.DRAG_COEFFICIENT = std::stod(m_edit_k->getText().toStdString());
        if (m_edit_F && !m_edit_F->getText().empty()) paramsFromUI.THRUST_COEFFICIENT = std::stod(m_edit_F->getText().toStdString());
    }
    catch (const std::exception& e) {
        std::cerr << "Error parsing input values: " << e.what() << std::endl;
        if (m_inputTitleLabel) m_inputTitleLabel->setText(L"������ ����� ����������!");
        m_trajectoryAvailable = false; m_calculatedStates.clear();
        prepareTrajectoryForDisplay(); populateTable({});
        return;
    }
    if (m_inputTitleLabel) m_inputTitleLabel->setText(L"�������� ��������");

    Calculations calculator;
    std::cout << "DEBUG: Running simulation with STEPS=" << paramsFromUI.STEPS
        << ", DT=" << paramsFromUI.DT << std::endl;

    m_calculatedStates = calculator.runSimulation(paramsFromUI);

    m_currentTableData.clear();
    if (!m_calculatedStates.empty()) {
        m_trajectoryAvailable = true;
        double currentTime = 0.0;
        const size_t maxTableEntries = 2000;
        size_t step = 1;
        
        if (m_calculatedStates.size() > maxTableEntries) {
            step = m_calculatedStates.size() / maxTableEntries;
            if (step == 0) step = 1; // �� ������, ���� calculatedStates.size() < maxTableEntries �� �� 0
        }

        for (size_t i = 0; i < m_calculatedStates.size(); i += step) {
            const auto& state = m_calculatedStates[i];
            m_currentTableData.push_back({
                static_cast<float>(i * paramsFromUI.DT), // ����� ������ �����
                static_cast<float>(state.x), static_cast<float>(state.y),
                static_cast<float>(state.vx), static_cast<float>(state.vy)
                });
        }
    }
    else {
        m_trajectoryAvailable = false;
    }

    prepareTrajectoryForDisplay(); // ���������� ������ � ��������� View ��� �������
    populateTable(m_currentTableData);
}

void UserInterface::prepareTrajectoryForDisplay() {
    m_trajectoryDisplayPoints.clear();
    if (!m_trajectoryAvailable || m_calculatedStates.empty()) {
        std::cout << "DEBUG: No trajectory to prepare for display." << std::endl;
        // ����� ������� clear, ����� ��� ��������� render �� ���������� ������ ����������
        // � ����� placeholder ����� ���������, ���� m_trajectoryAvailable == false
        return;
    }

    m_trajectoryDisplayPoints.reserve(m_calculatedStates.size());
    for (const auto& state : m_calculatedStates) {
        m_trajectoryDisplayPoints.emplace_back(
            sf::Vector2f(static_cast<float>(state.x), static_cast<float>(-state.y)), // Y ������������� ��� �����������
            sf::Color::Blue // ���� ����� ����������
        );
    }
    std::cout << "DEBUG: Trajectory display points prepared. Count: " << m_trajectoryDisplayPoints.size() << std::endl;
}

void UserInterface::drawTrajectoryOnCanvas(sf::RenderTarget& canvasRenderTarget) {
    sf::View trajectoryView;

    if (m_trajectoryAvailable && !m_trajectoryDisplayPoints.empty()) {
        float min_x = m_trajectoryDisplayPoints[0].position.x;
        float max_x = m_trajectoryDisplayPoints[0].position.x;
        float min_y = m_trajectoryDisplayPoints[0].position.y; // ��� -state.y
        float max_y = m_trajectoryDisplayPoints[0].position.y; // ��� -state.y

        for (const auto& vertex : m_trajectoryDisplayPoints) {
            min_x = std::min(min_x, vertex.position.x);
            max_x = std::max(max_x, vertex.position.x);
            min_y = std::min(min_y, vertex.position.y);
            max_y = std::max(max_y, vertex.position.y);
        }

        // ��������� ����������� ���� (0,0) � ������ ������, ���� ��� �� ������
        min_x = std::min(min_x, 0.0f);
        max_x = std::max(max_x, 0.0f);
        min_y = std::min(min_y, 0.0f); // ������� 0, �������� 0 (����� �������� Y)
        max_y = std::max(max_y, 0.0f);


        float worldWidth = max_x - min_x;
        float worldHeight = max_y - min_y;

        // ��������� �������, ����� ���������� �� ��������� � �����
        float paddingFactor = 0.1f; // 10% ������
        float paddingX = (worldWidth == 0) ? 1.0f : worldWidth * paddingFactor;
        float paddingY = (worldHeight == 0) ? 1.0f : worldHeight * paddingFactor;
        if (worldWidth == 0 && worldHeight == 0) { // ���� ����� ���� �����
            paddingX = 1.0f; paddingY = 1.0f; // ���� �����-�� ������ �������
        }


        sf::FloatRect viewRect(min_x - paddingX,
            min_y - paddingY,
            worldWidth + 2 * paddingX,
            worldHeight + 2 * paddingY);

        trajectoryView.reset(viewRect); // ������������� View �� ������ ������������� ��������������
        canvasRenderTarget.setView(trajectoryView);

        float centralBodyViewRadius = std::min(viewRect.width, viewRect.height) * 0.01f; // 1% �� ������� ������� View
        if (centralBodyViewRadius < 0.001f) centralBodyViewRadius = 0.001f; // ����������� ������

        sf::CircleShape centerBody(centralBodyViewRadius);
        centerBody.setFillColor(sf::Color::Red);
        centerBody.setOrigin(centralBodyViewRadius, centralBodyViewRadius);
        centerBody.setPosition(0.f, 0.f); // ������� ���������� (0,0)
        canvasRenderTarget.draw(centerBody);

        // ������ ����������
        if (m_trajectoryDisplayPoints.size() >= 1) {
            canvasRenderTarget.draw(m_trajectoryDisplayPoints.data(), m_trajectoryDisplayPoints.size(), sf::LineStrip);
        }

    }
    else {
        // ���� ��� ����������, ���������� ����������� ��� ������� ��� ������-��������
        canvasRenderTarget.setView(canvasRenderTarget.getDefaultView());
        sf::Text placeholderText;
        if (m_sfmlFont.hasGlyph(L'�')) { // ��������, ��� ����� �������� � ����� ���������
            placeholderText.setFont(m_sfmlFont);
            placeholderText.setString(L"���������� �� ����������.\n������� '���������� ����������!'");
        }
        else {
            placeholderText.setString("Trajectory not calculated.\nPress 'Calculate Trajectory!'");
            if (!m_sfmlFont.getInfo().family.empty()) // ���� ����� ��� ��������, �� �� ���
                std::cerr << "Warning: SFML font loaded but might not support Cyrillic for placeholder.\n";
        }
        placeholderText.setCharacterSize(16); // ������ � �������� ��� DefaultView
        placeholderText.setFillColor(sf::Color(105, 105, 105)); // DimGray
        sf::FloatRect textRect = placeholderText.getLocalBounds();
        placeholderText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
        placeholderText.setPosition(canvasRenderTarget.getSize().x / 2.0f, canvasRenderTarget.getSize().y / 2.0f);
        canvasRenderTarget.draw(placeholderText);
    }

    canvasRenderTarget.setView(canvasRenderTarget.getDefaultView());
}

void UserInterface::populateTable(const std::vector<TableRowData>& data) {
    if (!m_tableDataGrid) { std::cerr << "Error: m_tableDataGrid is null in populateTable!" << std::endl; return; }
    m_tableDataGrid->removeAllWidgets();

    if (data.empty()) {
        auto emptyLabel = tgui::Label::create(L"��� ������ ��� �����������");
        if (emptyLabel) {
            emptyLabel->setHorizontalAlignment(tgui::Label::HorizontalAlignment::Center);
            m_tableDataGrid->addWidget(emptyLabel, 0, 0);
            // ����� emptyLabel ������� ��� 5 �������, ���� API Grid �� ��������� colspan:
            // for (unsigned int j = 1; j < 5; ++j) {
            //     m_tableDataGrid->addWidget(tgui::Label::create(""), 0, j); // ������ �����
            // }
        }
        if (m_tableDataPanel) m_tableDataPanel->setContentSize({ 0,0 });
        return;
    }

    for (size_t i = 0; i < data.size(); ++i) {
        const auto& rowData = data[i];
        std::stringstream ss_h, ss_x, ss_y, ss_vx, ss_vy;
        ss_h << std::fixed << std::setprecision(2) << rowData.h_sec;
        ss_x << std::fixed << std::setprecision(2) << rowData.x;
        ss_y << std::fixed << std::setprecision(2) << rowData.y;
        ss_vx << std::fixed << std::setprecision(2) << rowData.Vx;
        ss_vy << std::fixed << std::setprecision(2) << rowData.Vy;
        std::vector<tgui::String> rowStrings = {
            tgui::String(ss_h.str()), tgui::String(ss_x.str()), tgui::String(ss_y.str()),
            tgui::String(ss_vx.str()), tgui::String(ss_vy.str()) };

        for (size_t j = 0; j < rowStrings.size(); ++j) {
            auto cellLabel = tgui::Label::create(rowStrings[j]);
            if (cellLabel) {
                cellLabel->getRenderer()->setTextColor(tgui::Color::Black);
                cellLabel->setHorizontalAlignment(tgui::Label::HorizontalAlignment::Center);
                m_tableDataGrid->addWidget(cellLabel, i, j);
                m_tableDataGrid->setWidgetPadding(i, j, { 2, 5, 2, 5 });
            }
        }
    }
    if (m_tableDataPanel && m_tableDataGrid) {
        m_tableDataPanel->setContentSize(m_tableDataGrid->getSize());
    }
}

// --- ������� ���� � ��������� ������� ---
void UserInterface::run() {
    m_window.setFramerateLimit(60); // ����������� FPS ��� ��������� � �������� ��������
    while (m_window.isOpen()) {
        handleEvents();
        update();
        render();
    }
}

void UserInterface::handleEvents() {
    sf::Event event;
    while (m_window.pollEvent(event)) {
        m_gui.handleEvent(event);
        if (event.type == sf::Event::Closed) {
            m_window.close();
        }
    }
}

void UserInterface::update() {
    // ��������, �������� ��� ������ ���������� ���������, �� ��������� � ������ ������������
}

void UserInterface::render() {
    if (m_trajectoryCanvas) {
        sf::RenderTexture& canvasRT = m_trajectoryCanvas->getRenderTexture();
        canvasRT.clear(sf::Color(250, 250, 250)); // ��� �������
        drawTrajectoryOnCanvas(canvasRT);      // ���� ����� ������ ��� ������������� � ���������� View
        m_trajectoryCanvas->display();
    }
    m_window.clear(sf::Color(220, 220, 220));
    m_gui.draw();
    m_window.display();
}