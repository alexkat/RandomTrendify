#include <wx/wx.h>
#include <wx/filedlg.h>
#include <wx/event.h>
#include "json.hpp"
#include <fstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <random>
#include <chrono>

using json = nlohmann::json;
using namespace std;

struct Segment
{
    int start;
    int end;
    string type; // "peak" or "trough"
};

std::vector<double> generateMarketTrendCurve(size_t length)
{
    std::vector<double> trend;
    if (length == 0)
    {
        return trend; // Return empty vector for zero length
    }

    trend.reserve(length); // Reserve space to avoid reallocations

    // Define a scaling factor to adjust the curve's frequency based on the list size
    double scale = std::max(1.0, std::log10(length));

    for (size_t i = 0; i < length; ++i)
    {
        // Adjust the sin function to scale with the list size
        double x = i / double(length) * scale * 2 * M_PI;
        trend.push_back(-std::sin(x) + 1);
    }

    return trend;
}

vector<Segment> localizePeaksAndTroughs(const vector<double> &trendCurve)
{
    vector<Segment> segments;
    Segment currentSegment{0, 0, trendCurve[0] < trendCurve[1] ? "peak" : "trough"};

    for (int i = 1; i < trendCurve.size() - 1; i++)
    {
        string currentType =
            (trendCurve[i - 1] > trendCurve[i] && trendCurve[i] < trendCurve[i + 1]) ? "peak" : (trendCurve[i - 1] < trendCurve[i] && trendCurve[i] > trendCurve[i + 1]) ? "trough"
                                                                                                                                                                         : currentSegment.type;

        if (currentType != currentSegment.type || i == trendCurve.size() - 2)
        {
            currentSegment.end = i;
            segments.push_back(currentSegment);
            currentSegment = {i, 0, currentType};
        }
    }
    currentSegment.end = trendCurve.size() - 1;
    segments.push_back(currentSegment);

    return segments;
}

std::mt19937 createSeededGenerator()
{
    unsigned seed = chrono::high_resolution_clock::now().time_since_epoch().count();
    random_device rd;
    seed_seq seq{rd(), rd(), rd(), static_cast<unsigned>(seed)};
    return mt19937(seq);
}

void shuffleSegment(json &array, int start, int end, std::mt19937 &generator)
{
    if (start < 0 || end > array.size() || start >= end)
    {
        throw std::invalid_argument("Invalid range for shuffleSegment");
    }
    for (int i = start; i < end; ++i)
    {
        std::uniform_int_distribution<> dis(i, end - 1);
        int j = dis(generator);
        std::swap(array[i], array[j]);
    }
}

json applyMarketTrendsAndRandomization(json &inputArray, std::mt19937 &generator)
{
    if (inputArray.empty())
        return json::array();

    // Calculate total tickets for normalization
    double totalTickets = 0;
    for (const auto &item : inputArray)
    {
        totalTickets += item["tickets"].get<double>();
    }

    // Create a cumulative distribution based on ticket counts
    std::vector<double> cumulativeWeights;
    double cumulative = 0;
    for (const auto &item : inputArray)
    {
        cumulative += item["tickets"].get<double>() / totalTickets;
        cumulativeWeights.push_back(cumulative);
    }

    // Shuffle using weighted probability
    json shuffledArray = json::array();
    std::uniform_real_distribution<double> distribution(0.0, 1.0);
    while (!inputArray.empty())
    {
        double randValue = distribution(generator);
        size_t selectedIndex = std::lower_bound(cumulativeWeights.begin(), cumulativeWeights.end(), randValue) - cumulativeWeights.begin();

        // Ensure selectedIndex is within the valid range after lower_bound
        selectedIndex = std::min(selectedIndex, inputArray.size() - 1);

        shuffledArray.push_back(inputArray[selectedIndex]);
        inputArray.erase(inputArray.begin() + selectedIndex);
        cumulativeWeights.erase(cumulativeWeights.begin() + selectedIndex);

        // Recalculate cumulativeWeights as the total has now changed
        cumulativeWeights.clear();
        cumulative = 0;
        totalTickets = 0;
        for (const auto &item : inputArray)
        {
            totalTickets += item["tickets"].get<double>();
        }
        for (const auto &item : inputArray)
        {
            cumulative += item["tickets"].get<double>() / totalTickets;
            cumulativeWeights.push_back(cumulative);
        }
    }

    return shuffledArray;
}

using json = nlohmann::json;

// Function declarations
std::vector<double> generateMarketTrendCurve(int length);
void shuffleSegment(json &array, int start, int end);
json applyMarketTrendsAndRandomization(json inputArray);

class MyApp : public wxApp
{
public:
    virtual bool OnInit();
};

class MyFrame : public wxFrame
{
public:
    MyFrame(const wxString &title);
    void OnDraw(wxPaintEvent &event);

private:
    void OnOpenFile(wxCommandEvent &event);
    void OnGenerate(wxCommandEvent &event);
    void OnSaveFile(wxCommandEvent &event);
    void OnTrimList(wxCommandEvent &event);
    json processInputFile(const std::string &path);
    json trimListToMaxParticipants(const json &list, size_t maxParticipants);

    wxPanel *drawingPanel;
    wxTextCtrl *logTextCtrl;
    wxTextCtrl *maxParticipantsInput; // Input for maximum number of participants
    wxButton *trimListButton;
    json generatedJson;           // Store the generated JSON data
    std::string selectedFilePath; // Store the path of the selected file
};

enum
{
    ID_Open = wxID_HIGHEST + 1,
    ID_Generate,
    ID_Save
};

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit()
{
    MyFrame *frame = new MyFrame("RandomTrendify");
    frame->Show(true);
    return true;
}

MyFrame::MyFrame(const wxString &title)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(600, 400))
{
    wxPanel *panel = new wxPanel(this, wxID_ANY);

    maxParticipantsInput = new wxTextCtrl(panel, wxID_ANY);
    trimListButton = new wxButton(panel, wxID_ANY, wxT("Trim List"));

    wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);

    wxButton *openBtn = new wxButton(panel, ID_Open, wxT("Open File"));
    wxButton *genBtn = new wxButton(panel, ID_Generate, wxT("Generate"));
    wxButton *saveBtn = new wxButton(panel, ID_Save, wxT("Save"));

    drawingPanel = new wxPanel(panel, wxID_ANY, wxDefaultPosition, wxSize(600, 200));
    drawingPanel->Bind(wxEVT_PAINT, &MyFrame::OnDraw, this);

    logTextCtrl = new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);

    hbox->Add(openBtn, 0, wxRIGHT, 5);
    hbox->Add(genBtn, 0, wxRIGHT, 5);
    hbox->Add(saveBtn, 0);
    hbox->Add(maxParticipantsInput, 0, wxRIGHT, 5);
    hbox->Add(trimListButton, 0, wxRIGHT, 5);

    vbox->Add(hbox, 0, wxALIGN_CENTER | wxALL, 10);
    vbox->Add(logTextCtrl, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    vbox->Add(drawingPanel, 1, wxEXPAND | wxALL, 5);

    panel->SetSizer(vbox);

    // Event bindings
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MyFrame::OnOpenFile, this, ID_Open);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MyFrame::OnGenerate, this, ID_Generate);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MyFrame::OnSaveFile, this, ID_Save);
    trimListButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MyFrame::OnTrimList, this);
}

void MyFrame::OnTrimList(wxCommandEvent &event)
{
    if (generatedJson.empty())
    {
        wxMessageBox(wxT("No data available to trim."), wxT("Error"), wxICON_ERROR);
        return;
    }

    long maxParticipants = 0;
    maxParticipantsInput->GetValue().ToLong(&maxParticipants);
    if (maxParticipants <= 0)
    {
        wxMessageBox(wxT("Please enter a valid maximum number of participants."), wxT("Error"), wxICON_ERROR);
        return;
    }

    generatedJson = trimListToMaxParticipants(generatedJson, static_cast<size_t>(maxParticipants));
    logTextCtrl->SetValue(wxT("List trimmed. New size: ") + wxString::Format(wxT("%lu"), generatedJson.size()));
}

json MyFrame::trimListToMaxParticipants(const json &list, size_t maxParticipants)
{
    if (list.size() <= maxParticipants)
        return list;

    json trimmedList = json::array();
    std::copy_n(list.begin(), maxParticipants, std::back_inserter(trimmedList));
    return trimmedList;
}

void MyFrame::OnOpenFile(wxCommandEvent &event)
{
    wxFileDialog openFileDialog(this, _("Open JSON file"), "", "",
                                "JSON files (*.json)|*.json", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (openFileDialog.ShowModal() == wxID_CANCEL)
        return; // User canceled

    selectedFilePath = openFileDialog.GetPath().ToStdString();
    logTextCtrl->AppendText("File selected: " + openFileDialog.GetPath() + "\n");
}

void MyFrame::OnGenerate(wxCommandEvent &event)
{
    if (selectedFilePath.empty())
    {
        logTextCtrl->AppendText("Please select a file first.\n");
        return;
    }

    try
    {
        generatedJson = processInputFile(selectedFilePath);
        logTextCtrl->Clear(); // Clear the log window before appending new output
        logTextCtrl->AppendText("Generated output:\n" + generatedJson.dump(4) + "\n");
        drawingPanel->Refresh();
    }
    catch (const std::exception &e)
    {
        logTextCtrl->Clear(); // Optionally clear the log before showing error
        logTextCtrl->AppendText(wxString("Error: ") + e.what() + "\n");
    }
}

void MyFrame::OnSaveFile(wxCommandEvent &event)
{
    if (generatedJson.is_null())
    {
        logTextCtrl->AppendText("No data to save. Please generate data first.\n");
        return;
    }

    wxFileDialog saveFileDialog(this, _("Save JSON file"), "", "",
                                "JSON files (*.json)|*.json", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveFileDialog.ShowModal() == wxID_CANCEL)
        return; // User canceled

    std::ofstream outputFile(saveFileDialog.GetPath().ToStdString());
    if (!outputFile)
    {
        logTextCtrl->AppendText("Failed to open the file for writing.\n");
        return;
    }
    outputFile << generatedJson.dump(4);
    logTextCtrl->AppendText("Data successfully saved to: " + saveFileDialog.GetPath() + "\n");
}

void MyFrame::OnDraw(wxPaintEvent &event)
{
    wxPaintDC dc(drawingPanel);
    dc.SetPen(wxPen(wxColor(0, 0, 255), 2)); // Blue pen of width 2

    if (!generatedJson.is_array() || generatedJson.empty())
        return;

    auto size = drawingPanel->GetSize();
    int width = size.GetWidth();
    int height = size.GetHeight();

    // Determine max "tickets" value for scaling
    double maxTicketsValue = 0;
    for (const auto &item : generatedJson)
    {
        double tickets = item["tickets"];
        if (tickets > maxTicketsValue)
        {
            maxTicketsValue = tickets;
        }
    }

    // Draw the curve based on "tickets" values
    for (size_t i = 1; i < generatedJson.size(); ++i)
    {
        int x1 = ((i - 1) * width) / generatedJson.size();
        int y1 = height - (generatedJson[i - 1]["tickets"].get<double>() / maxTicketsValue * (height - 20));
        int x2 = (i * width) / generatedJson.size();
        int y2 = height - (generatedJson[i]["tickets"].get<double>() / maxTicketsValue * (height - 20));
        dc.DrawLine(x1, y1 + 10, x2, y2 + 10); // +10 to leave some margin
    }
}

json MyFrame::processInputFile(const std::string &path)
{
    std::ifstream inputFile(path);
    if (!inputFile)
    {
        throw std::runtime_error("Could not open the input file.");
    }

    json inputArray;
    try
    {
        inputFile >> inputArray;
    }
    catch (json::parse_error &e)
    {
        // Handle JSON parsing error (e.g., malformed JSON)
        std::cerr << "JSON parsing error: " << e.what() << '\n';
        throw;
    }

    if (!inputArray.is_array())
    {
        // Handle the case where the JSON content is not an array as expected
        throw std::runtime_error("Expected an array of objects in the input file.");
    }

    // Ensure the generator is seeded only once and used throughout the application lifecycle
    static std::mt19937 generator = createSeededGenerator();

    // Apply market trends and randomization algorithm to the input array
    return applyMarketTrendsAndRandomization(inputArray, generator);
}
